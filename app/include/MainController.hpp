//
// Created by nikola on 23.11.25..
//

#ifndef MATF_RG_PROJECT_MAINCONTROLLER_HPP
#define MATF_RG_PROJECT_MAINCONTROLLER_HPP
#include "engine/core/Controller.hpp"

namespace app {

class MainController final : public engine::core::Controller {
    void initialize() override;
    bool loop() override;
    static void draw_saturn();
    void begin_draw() override;
    void end_draw() override;
    void update_camera();
    void update() override;
    void draw() override;

public:
    std::string_view name() const override {
    return "app::MainController";
    }
};

}// namespace app

#endif//MATF_RG_PROJECT_MAINCONTROLLER_HPP
