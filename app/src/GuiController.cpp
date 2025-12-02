#include "GuiController.hpp"
#include "MainController.hpp"
#include "engine/graphics/GraphicsController.hpp"
#include "engine/platform/PlatformController.hpp"
#include "imgui.h"

namespace app {

void GuiController::initialize() {
    set_enable(false);
}

void GuiController::poll_events() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();

    if (platform->key(engine::platform::KeyId::KEY_ENTER).state() == engine::platform::Key::State::JustPressed)
        set_enable(!is_enabled());
}

void GuiController::draw() {
    auto main_controller = engine::core::Controller::get<MainController>();
    const auto& params = main_controller->get_scene_params();

    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    graphics->begin_gui();

    ImGui::Begin("Settings");
    ImGui::Text("Directional light intensity: %.2f", params.dir_light_intensity);
    ImGui::Text("Saturn visible: %s", params.saturn_visible ? "true" : "false");
    ImGui::End();
    graphics->end_gui();
}

} // namespace app
