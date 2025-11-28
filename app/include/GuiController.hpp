//
// Created by nikola on 23.11.25..
//

#ifndef MATF_RG_PROJECT_GUICONTROLLER_HPP
#define MATF_RG_PROJECT_GUICONTROLLER_HPP
#include "engine/core/Controller.hpp"

namespace app {

class GuiController : public engine::core::Controller {
public:
    std::string_view name() const override {
        return "app::GuiController";
    }
    float getDirLightIntensity() const { return dirLightIntensity; }
    bool getSaturnVisible() const { return saturnVisible; }
    void setSaturnVisible(bool value) { saturnVisible = value; }

    bool getSaturnToggleRequested() const { return saturnToggleRequested; }
    void setSaturnToggleRequested(bool value) { saturnToggleRequested = value; }

    float getSaturnToggleTimer() const { return saturnToggleTimer; }
    void setSaturnToggleTimer(float value) { saturnToggleTimer = value; }

private:
    float dirLightIntensity = 1.0f;

    bool saturnVisible = true;

    bool saturnToggleRequested = false;

    float saturnToggleTimer = 0.0f;


    void initialize() override;

    void poll_events() override;

    void draw() override;
};

}// namespace app

#endif//MATF_RG_PROJECT_GUICONTROLLER_HPP
