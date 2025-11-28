#include "../include/GuiController.hpp"
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

    if (platform->key(engine::platform::KeyId::KEY_Q).state() == engine::platform::Key::State::JustPressed) {
        dirLightIntensity += 0.1f;
        if (dirLightIntensity > 5.0f) dirLightIntensity = 5.0f;
    }
    if (platform->key(engine::platform::KeyId::KEY_E).state() == engine::platform::Key::State::JustPressed) {
        dirLightIntensity -= 0.1f;
        if (dirLightIntensity < 0.0f) dirLightIntensity = 0.0f;
    }
    if (platform->key(engine::platform::KeyId::KEY_R).state() == engine::platform::Key::State::JustPressed) {
        if (!saturnToggleRequested) {
            saturnToggleRequested = true;
            saturnToggleTimer = 0.0f;
        }
    }

    if (saturnToggleRequested) {
        float dt = platform->dt();
        saturnToggleTimer += dt;
        if (saturnToggleTimer >= 3.0f) {
            saturnVisible = !saturnVisible;
            saturnToggleRequested = false;
            saturnToggleTimer = 0.0f;
        }
    }
}


void GuiController::draw() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    graphics->begin_gui();

    ImGui::Begin("Settings");

    ImGui::Text("Directional light intensity: %.2f", dirLightIntensity);
    ImGui::Text("Saturn visible: %s", saturnVisible ? "true" : "false");


    ImGui::End();
    graphics->end_gui();
}


}// namespace app
