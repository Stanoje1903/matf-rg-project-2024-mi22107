//
// Created by nikola on 23.11.25..
//

#include "../include/MainController.hpp"

#include "../../engine/test/app/include/app/GUIController.hpp"
#include "GuiController.hpp"
#include "engine/graphics/GraphicsController.hpp"
#include "engine/graphics/OpenGL.hpp"
#include "engine/platform/PlatformController.hpp"
#include "engine/resources/ResourcesController.hpp"

namespace app {
class MainPlatformEventObserver final : public engine::platform::PlatformEventObserver {
public:
    void on_mouse_move(engine::platform::MousePosition position) override;
};
void MainPlatformEventObserver::on_mouse_move(engine::platform::MousePosition position) {
    auto gui_controller = engine::core::Controller::get<GuiController>();
    if (!gui_controller->is_enabled()) {
        auto camera = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();
        camera->rotate_camera(position.dx, position.dy);
    }
}

void MainController::initialize() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    platform->register_platform_event_observer(std::make_unique<MainPlatformEventObserver>());
    engine::graphics::OpenGL::enable_depth_testing();
}

bool MainController::loop() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KeyId::KEY_ESCAPE).is_down()) {
        return false;
    }
    return true;
}
void MainController::draw_jupiter() {
    static float jupiterRotation = 0.0f;
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    engine::resources::Model *jupiter = resources->model("jupiter");
    engine::resources::Shader *shader = resources->shader("multiple_lights");
    shader->use();

    float dt = engine::core::Controller::get<engine::platform::PlatformController>()->dt();
    jupiterRotation += dt * 5.0f;
    if (jupiterRotation > 360.0f) jupiterRotation -= 360.0f;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0f));
    model = glm::rotate(model, glm::radians(jupiterRotation), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.3f));

    shader->set_vec3("viewPos", graphics->camera()->Position);

    shader->set_vec3("dirLight.direction", glm::vec3(-0.3f, -1.0f, -0.3f));
    shader->set_vec3("dirLight.ambient", glm::vec3(0.05f));
    shader->set_vec3("dirLight.diffuse", glm::vec3(1.0f));
    shader->set_vec3("dirLight.specular", glm::vec3(0.5f));

    shader->set_vec3("pointLight.position", glm::vec3(0.0f, 0.0f, -3.0f));
    shader->set_vec3("pointLight.ambient", glm::vec3(0.05f));
    shader->set_vec3("pointLight.diffuse", glm::vec3(0.05f));
    shader->set_vec3("pointLight.specular", glm::vec3(0.05f));
    shader->set_float("pointLight.constant", 1.0f);
    shader->set_float("pointLight.linear", 0.09f);
    shader->set_float("pointLight.quadratic", 0.032f);

    shader->set_float("material.shininess", 32.0f);

    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    shader->set_mat4("model", model);

    jupiter->draw(shader);
}

void MainController::draw_saturn() {
    static float saturnRotation = 0.0f;
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();

    engine::resources::Model* saturn = resources->model("saturn");
    engine::resources::Shader* shader = resources->shader("multiple_lights");
    shader->use();

    auto camera = graphics->camera();
    shader->set_vec3("viewPos", camera->Position);

    shader->set_vec3("dirLight.direction", glm::vec3(-0.3f, -1.0f, -0.3f));
    shader->set_vec3("dirLight.ambient",  glm::vec3(0.02f));
    shader->set_vec3("dirLight.diffuse",  glm::vec3(0.7f));
    shader->set_vec3("dirLight.specular", glm::vec3(1.0f));

    glm::vec3 jupiterPos = glm::vec3(0.0f, 0.0f, -3.0f);
    float dt = engine::core::Controller::get<engine::platform::PlatformController>()->dt();
    saturnRotation += dt * 20.0f;
    if (saturnRotation > 360.0f) saturnRotation -= 360.0f;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, jupiterPos);
    model = glm::rotate(model, glm::radians(saturnRotation), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.001f));

    shader->set_vec3("pointLight.position", glm::vec3(model[3]));
    shader->set_vec3("pointLight.ambient",  glm::vec3(0.01f));
    shader->set_vec3("pointLight.diffuse",  glm::vec3(0.01f));
    shader->set_vec3("pointLight.specular", glm::vec3(0.02f));
    shader->set_float("pointLight.constant", 1.0f);
    shader->set_float("pointLight.linear", 0.09f);
    shader->set_float("pointLight.quadratic", 0.032f);

    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    shader->set_mat4("model", model);

    shader->set_int("material.diffuse", 0);
    shader->set_int("material.specular", 1);
    shader->set_float("material.shininess", 32.0f);

    saturn->draw(shader);
}
void MainController::draw_skybox() {
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto skybox = resources->skybox("night_skybox");
    auto shader = resources->shader("skybox");
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    graphics->draw_skybox(shader, skybox);
}

void MainController::begin_draw() {
    engine::graphics::OpenGL::clear_buffers();
}

void MainController::draw() {
    draw_jupiter();
    draw_saturn();
    draw_skybox();
}

void MainController::end_draw() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    platform->swap_buffers();
}

void MainController::update() {
    update_camera();
}

void MainController::update_camera() {
    auto gui_controller = engine::core::Controller::get<GuiController>();
    if (gui_controller->is_enabled()) {
        return;
    }
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto camera = graphics->camera();
    float dt = platform->dt();
    if (platform->key(engine::platform::KeyId::KEY_W).is_down()) {
        camera->move_camera(engine::graphics::Camera::Movement::FORWARD, dt);
    }
    if (platform->key(engine::platform::KeyId::KEY_A).is_down()) {
        camera->move_camera(engine::graphics::Camera::Movement::LEFT, dt);
    }
    if (platform->key(engine::platform::KeyId::KEY_S).is_down()) {
        camera->move_camera(engine::graphics::Camera::Movement::BACKWARD, dt);
    }
    if (platform->key(engine::platform::KeyId::KEY_D).is_down()) {
        camera->move_camera(engine::graphics::Camera::Movement::RIGHT, dt);
    }
}

}// namespace app