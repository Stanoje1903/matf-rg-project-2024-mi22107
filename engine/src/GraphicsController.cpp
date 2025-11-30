
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/Skybox.hpp>
#include <engine/resources/ResourcesController.hpp>

namespace engine::graphics {

void GraphicsController::initialize_bloom() {
    auto platform = engine::core::Controller::get<platform::PlatformController>();

    int width = platform->window()->width();
    int height = platform->window()->height();

    glGenFramebuffers(1, &m_hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);

    glGenTextures(1, &m_hdrColorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_hdrColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_hdrColorBuffer, 0);

    glGenRenderbuffers(1, &m_hdrRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_hdrRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_hdrRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("HDR FBO not complete!");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(2, m_pingpongFBO);
    glGenTextures(2, m_pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, m_pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pingpongColorbuffers[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Pingpong FBO not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    m_bloomExtractShader = resources->shader("bloom_extract");
    m_blurShader = resources->shader("bloom_blur");
    m_bloomCombineShader = resources->shader("bloom_final");

    if (m_bloomExtractShader) { m_bloomExtractShader->use(); m_bloomExtractShader->set_int("scene", 0); }
    if (m_blurShader) { m_blurShader->use(); m_blurShader->set_int("image", 0); }
    if (m_bloomCombineShader) { m_bloomCombineShader->use(); m_bloomCombineShader->set_int("scene", 0); m_bloomCombineShader->set_int("bloomBlur", 1); }

    m_quadVAO = 0;
}

void GraphicsController::bind_hdr_fbo() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);
}

void GraphicsController::unbind_hdr_fbo() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsController::render_quad() {
    if (m_quadVAO == 0) {
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
        };
        glGenVertexArrays(1, &m_quadVAO);
        glGenBuffers(1, &m_quadVBO);
        glBindVertexArray(m_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void GraphicsController::apply_bloom() {
    unsigned int sceneTex = (m_hdrColorBuffer != 0 ? m_hdrColorBuffer : m_resolveColor);
    if (sceneTex == 0) {
        return;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTex);

    glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[0]);
    glClear(GL_COLOR_BUFFER_BIT);
    m_bloomExtractShader->use();
    m_bloomExtractShader->set_int("scene", 0);
    m_bloomExtractShader->set_float("threshold", 0.8f);
    render_quad();

    bool horizontal = true;
    bool first_iter = true;
    for (int i = 0; i < m_bloomBlurIterations; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[horizontal ? 1 : 0]);
        m_blurShader->use();
        m_blurShader->set_bool("horizontal", horizontal);
        // source texture:
        unsigned int srcTex = first_iter ? m_pingpongColorbuffers[0] : m_pingpongColorbuffers[horizontal ? 0 : 1];
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, srcTex);
        render_quad();

        horizontal = !horizontal;
        if (first_iter) first_iter = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int finalBlurTex = m_pingpongColorbuffers[ horizontal ? 0 : 1 ];

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_bloomCombineShader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTex);
    m_bloomCombineShader->set_int("scene", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, finalBlurTex);
    m_bloomCombineShader->set_int("bloomBlur", 1);

    m_bloomCombineShader->set_float("exposure", 1.0f);
    m_bloomCombineShader->set_float("bloomStrength", 1.0f);

    render_quad();
}

void GraphicsController::resize_bloom(int width, int height)
{
    glDeleteFramebuffers(1, &m_hdrFBO);
    glDeleteTextures(1, &m_hdrColorBuffer);
    glDeleteRenderbuffers(1, &m_hdrRBO);

    glGenFramebuffers(1, &m_hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);

    glGenTextures(1, &m_hdrColorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_hdrColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_hdrColorBuffer, 0);

    glGenRenderbuffers(1, &m_hdrRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_hdrRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_hdrRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    for (int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, m_pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pingpongColorbuffers[i], 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void GraphicsController::initialize_msaa(int samples) {
    m_msaaSamples = samples;
    auto platform = engine::core::Controller::get<platform::PlatformController>();
    int width = platform->window()->width();
    int height = platform->window()->height();

    m_msaaFBO = OpenGL::create_msaa_fbo(
        samples, width, height,
        m_msaaColor, m_msaaDepth
    );

    m_resolveFBO = OpenGL::create_resolve_fbo(
        width, height,
        m_resolveColor
    );
}
void GraphicsController::bind_msaa_fbo() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFBO);
}
void GraphicsController::resolve_msaa_and_present() {
    auto platform = engine::core::Controller::get<platform::PlatformController>();
    OpenGL::blit_msaa_to_screen(
    m_msaaFBO,
    m_resolveFBO,
    platform->window()->width(),
    platform->window()->height()
);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_resolveFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glBlitFramebuffer(
        0, 0, platform->window()->width(), platform->window()->height(),
        0, 0, platform->window()->width(), platform->window()->height(),
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST
    );
}
void GraphicsController::resize_msaa() {
    auto platform = engine::core::Controller::get<platform::PlatformController>();
    int width = platform->window()->width();
    int height = platform->window()->height();

    if (m_msaaFBO != 0) {
        glDeleteFramebuffers(1, &m_msaaFBO);
        glDeleteTextures(1, &m_msaaColor);
        glDeleteRenderbuffers(1, &m_msaaDepth);
    }

    if (m_resolveFBO != 0) {
        glDeleteFramebuffers(1, &m_resolveFBO);
        glDeleteTextures(1, &m_resolveColor);
    }

    m_msaaFBO = OpenGL::create_msaa_fbo(
        m_msaaSamples, width, height,
        m_msaaColor, m_msaaDepth
    );

    m_resolveFBO = OpenGL::create_resolve_fbo(
        width, height,
        m_resolveColor
    );
}

void GraphicsController::resolve_msaa_to_hdr() {
    auto platform = engine::core::Controller::get<platform::PlatformController>();
    int width = platform->window()->width();
    int height = platform->window()->height();

    if (m_msaaFBO == 0) return;
    if (m_hdrFBO == 0) {
        if (m_resolveFBO != 0) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaaFBO);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_resolveFBO);
            glBlitFramebuffer(0,0,width,height,0,0,width,height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        return;
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_hdrFBO);
    glBlitFramebuffer(
        0, 0, width, height,
        0, 0, width, height,
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
        GL_NEAREST
    );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void GraphicsController::initialize() {
    const int opengl_initialized = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    RG_GUARANTEE(opengl_initialized, "OpenGL failed to init!");

    auto platform = engine::core::Controller::get<platform::PlatformController>();
    auto handle = platform->window()
                          ->handle_();
    m_perspective_params.FOV = glm::radians(m_camera.Zoom);
    m_perspective_params.Width = static_cast<float>(platform->window()
                                                            ->width());
    m_perspective_params.Height = static_cast<float>(platform->window()
                                                             ->height());
    m_perspective_params.Near = 0.1f;
    m_perspective_params.Far = 100.f;

    m_ortho_params.Bottom = 0.0f;
    m_ortho_params.Top = static_cast<float>(platform->window()
                                                    ->height());
    m_ortho_params.Left = 0.0f;
    m_ortho_params.Right = static_cast<float>(platform->window()
                                                      ->width());
    m_ortho_params.Near = 0.1f;
    m_ortho_params.Far = 100.0f;
    platform->register_platform_event_observer(
            std::make_unique<GraphicsPlatformEventObserver>(this));
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    RG_GUARANTEE(ImGui_ImplGlfw_InitForOpenGL(handle, true), "ImGUI failed to initialize for OpenGL");
    RG_GUARANTEE(ImGui_ImplOpenGL3_Init("#version 330 core"), "ImGUI failed to initialize for OpenGL");
}

void GraphicsController::terminate() {
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

void GraphicsPlatformEventObserver::on_window_resize(int width, int height) {
    m_graphics->perspective_params()
              .Width = static_cast<float>(width);
    m_graphics->perspective_params()
              .Height = static_cast<float>(height);

    m_graphics->orthographic_params()
              .Right = static_cast<float>(width);
    m_graphics->orthographic_params()
              .Top = static_cast<float>(height);
    m_graphics->resize_msaa();
    m_graphics->resize_bloom(width, height);
}

std::string_view GraphicsController::name() const {
    return "GraphicsController";
}

void GraphicsController::begin_gui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GraphicsController::end_gui() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GraphicsController::draw_skybox(const resources::Shader *shader, const resources::Skybox *skybox) {
    glm::mat4 view = glm::mat4(glm::mat3(m_camera.view_matrix()));
    shader->use();
    shader->set_mat4("view", view);
    shader->set_mat4("projection", projection_matrix<>());
    CHECKED_GL_CALL(glDepthFunc, GL_LEQUAL);
    CHECKED_GL_CALL(glBindVertexArray, skybox->vao());
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, skybox->texture());
    CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 36);
    CHECKED_GL_CALL(glBindVertexArray, 0);
    CHECKED_GL_CALL(glDepthFunc, GL_LESS); // set depth function back to default
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, 0);
}
}
