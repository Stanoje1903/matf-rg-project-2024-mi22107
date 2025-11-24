//
// Created by nikola on 23.11.25..
//

#include "../include//MyApp.hpp"

#include "MainController.hpp"
#include "spdlog/spdlog.h"

#include <any>

namespace app {
    void MyApp::app_setup() {
    spdlog::info("App setup");
    auto main_controller = register_controller<app::MainController>();
    main_controller->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());

    }

}// namespace app