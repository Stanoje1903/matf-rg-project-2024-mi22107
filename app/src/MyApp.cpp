//
// Created by nikola on 23.11.25..
//

#include "MyApp.hpp"
#include "GuiController.hpp"
#include "MainController.hpp"
#include "spdlog/spdlog.h"

namespace app {
    void MyApp::app_setup() {
    spdlog::info("App setup");
    auto main_controller = register_controller<app::MainController>();
    auto gui_controller = register_controller<app::GuiController>();
    main_controller->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
    main_controller->before(gui_controller);
    }

}// namespace app