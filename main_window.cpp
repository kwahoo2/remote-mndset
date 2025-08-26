// SPDX-License-Identifier: BSL-1.0
/*!
 * @author Adrian Przekwas <adrian.v.przekwas@gmail.com>
 */

#include "imgui.h"
#include "main_window.h"

void drawMainWindow(WindowState& state) {
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    // Here define the contents of window
    ImGui::Begin("Monado remote controller");
    if (state.iCons == hmd) {
        ImGui::Text("HMD placement input is active");
    } else if (state.iCons == left_controller) {
        ImGui::Text("Left Controller placement input is active");
    } else if (state.iCons == right_controller) {
        ImGui::Text("Right Controller placement input is active");
    }
    if (state.has_gamepad) {
        std::string b_str = "";
        if (state.batt >= 0) {
            b_str = ", Battery level: " + std::to_string(state.batt) + "%";
        }
        ImGui::Text("Gamepad available: %s%s", state.gamepad_name.c_str(), b_str.c_str());
    } else {
        ImGui::Text("No gamepad found");
    }
    if (state.connect_button_clicked){
        if (ImGui::Button("Disconnect")){
            state.connect_button_clicked = false;
        }
    }
    else {
        if (ImGui::Button("Connect")){
            state.connect_button_clicked = true;
        }
    }
    if (state.grab_button_clicked) {
        ImGui::Button("Press Esc to release mouse and keyboard");
    } else {
        if (ImGui::Button("Grab mouse and keyboard")) {
            state.grab_button_clicked = true;
        }
    }
    // Preferences
    ImGui::PushItemWidth(-300);
    ImGui::SliderFloat("HMD linear movement velocity", &state.config.hmd_lin_vel, 0.0f, 10.0f);
    ImGui::SliderFloat("HMD angular movement velocity", &state.config.hmd_ang_vel, 0.0f, 10.0f);
    ImGui::SliderFloat("Controllers linear movement velocity", &state.config.controller_lin_vel, 0.0f, 10.0f);
    ImGui::SliderFloat("Controllers angular movement velocity", &state.config.controller_ang_vel, 0.0f, 10.0f);
    ImGui::SliderFloat("Mouse sensivity", &state.config.mouse_sens, 0.0f, 5.0f);
    ImGui::SliderFloat("Gamepad axis sensivity", &state.config.gamepad_axis_sens, 0.0f, 5.0f);
    ImGui::SliderFloat("Gamepad dead zone", &state.config.gamepad_dead_zone, 0.0f, 0.5f);
    ImGui::End();
}
