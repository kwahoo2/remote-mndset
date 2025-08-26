// SPDX-License-Identifier: BSL-1.0
/*!
 * @author Adrian Przekwas <adrian.v.przekwas@gmail.com>
 */

#include "movement.h"
#include "math_helper.h"

#include <cstdlib>

Movement::Movement() {
    lin_vel = 0.001f; // linear velocity
    ang_vel = 0.001f; // angular velocity
    mouse_sens = 0.5f; // mouse sensivity
    gamepad_axis_sens = 1.0f;
    gamepad_dead_zone = 0.1f;
    r_rate_mod = 16.0f; // frame duration-dependent velocity correction
    old_ticks = 0;
}
Movement::~Movement() {

}

/* Adjust user settings */
void Movement::updateConfigValues(float lin_v, float ang_v,
                                  float mouse_s, float g_axis_sens, float g_dead_zone) {
    lin_vel = lin_v / 1000.0f;
    ang_vel = ang_v / 1000.0f;
    mouse_sens = mouse_s;
    gamepad_axis_sens = g_axis_sens;
    gamepad_dead_zone = g_dead_zone;
}

/* Pass keboard press/release keys events, and modify movement speed and actions*/
void Movement::passKeyboardEvent(SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_UP){
        checkKeyUp(event.key.key);
    }
    else if (event.type == SDL_EVENT_KEY_DOWN){
        checkKeyDown(event.key.key);
    }
}

/* Pass how much cursor moved since the last frame */
void Movement::passMouseRelativePos(float x, float y) {
    mov_mod.yaw = -x * mouse_sens;
    mov_mod.pitch = -y * mouse_sens;
}

void Movement::passGamepadState(SDL_Gamepad& gamepad){
    Sint16 left_x = SDL_GetGamepadAxis(&gamepad, SDL_GAMEPAD_AXIS_LEFTX);
    Sint16 left_y = SDL_GetGamepadAxis(&gamepad, SDL_GAMEPAD_AXIS_LEFTY);
    Sint16 right_x = SDL_GetGamepadAxis(&gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
    Sint16 right_y = SDL_GetGamepadAxis(&gamepad, SDL_GAMEPAD_AXIS_RIGHTY);

    float left_x_f = static_cast<float>(left_x) / gamepad_axis_range;
    if (std::abs(left_x_f) > gamepad_dead_zone) {
        mov_mod.sidestep = left_x_f * gamepad_axis_sens;
    } else {
        mov_mod.sidestep = 0.0f;
    }
    float left_y_f = static_cast<float>(left_y) / gamepad_axis_range;
    if (std::abs(left_y_f) > gamepad_dead_zone) {
        mov_mod.walk = left_y_f * gamepad_axis_sens;
    } else {
        mov_mod.walk = 0.0f;
    }
    float right_x_f = static_cast<float>(right_x) / gamepad_axis_range;
    if (std::abs(right_x_f) > gamepad_dead_zone) {
        mov_mod.yaw = -right_x_f * gamepad_axis_sens;
    } else {
        mov_mod.yaw = 0.0f;
    }
    float right_y_f = static_cast<float>(right_y) / gamepad_axis_range;
    if (std::abs(right_y_f) > gamepad_dead_zone) {
        mov_mod.pitch = -right_y_f * gamepad_axis_sens;
    } else {
        mov_mod.pitch = 0.0f;
    }

    if (SDL_GetGamepadButton(&gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP)) {
        mov_mod.altitude = 1.0f;
    } else if (SDL_GetGamepadButton(&gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN)) {
        mov_mod.altitude = -1.0f;
    } else {
        mov_mod.altitude = 0.0f;
    }
    if (SDL_GetGamepadButton(&gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT)) {
        mov_mod.roll = 1.0f;
    } else if (SDL_GetGamepadButton(&gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT)) {
        mov_mod.roll = -1.0f;
    } else {
        mov_mod.roll = 0.0f;
    }

}

/* Ticks are used for frame time calculation and adjusting movement speed in every step */
void Movement::updateTicks(Uint64 ticks) {
    Uint64 frame_ticks = ticks - old_ticks;
    r_rate_mod = static_cast<float>(frame_ticks);
    old_ticks = ticks;
}

void Movement::updatePose(xrt_pose& pose) {
    auto [yaw, pitch, roll] = quatToYXZ(pose.orientation);

    yaw += mov_mod.yaw * ang_vel * r_rate_mod;
    pitch += mov_mod.pitch * ang_vel * r_rate_mod;
    roll += mov_mod.roll * ang_vel * r_rate_mod;
    pose.orientation = quatFromYXZ(yaw, pitch, roll);

    xrt_vec3 delta_pos = { mov_mod.sidestep * lin_vel * r_rate_mod,
                           mov_mod.altitude * lin_vel * r_rate_mod,
                           mov_mod.walk * lin_vel * r_rate_mod };
    delta_pos = quatMultVec(pose.orientation, delta_pos);
    pose.position = pose.position + delta_pos;
}

void Movement::checkKeyDown(SDL_Keycode key) {
    switch (key){
        case SDLK_W:
            mov_mod.walk = -1.0f;
            break;
        case SDLK_S:
            mov_mod.walk = 1.0f;
            break;
        case SDLK_A:
            mov_mod.sidestep = -1.0f;
            break;
        case SDLK_D:
            mov_mod.sidestep = 1.0f;
            break;
        case SDLK_Q:
            mov_mod.roll = 1.0f;
            break;
        case SDLK_E:
            mov_mod.roll = -1.0f;
            break;
        case SDLK_LCTRL:
            mov_mod.altitude = -1.0f;
            break;
        case SDLK_SPACE:
            mov_mod.altitude = 1.0f;
            break;
    }
}

void Movement::checkKeyUp(SDL_Keycode key) {
    switch (key){
    case SDLK_W:
        mov_mod.walk = 0.0f;
        break;
    case SDLK_S:
        mov_mod.walk = 0.0f;
        break;
    case SDLK_A:
        mov_mod.sidestep = 0.0f;
        break;
    case SDLK_D:
        mov_mod.sidestep = 0.0f;
        break;
    case SDLK_Q:
        mov_mod.roll = 0.0f;
        break;
    case SDLK_E:
        mov_mod.roll = 0.0f;
        break;
    case SDLK_LCTRL:
        mov_mod.altitude = 0.0f;
        break;
    case SDLK_SPACE:
        mov_mod.altitude = 0.0f;
        break;
    }
}
