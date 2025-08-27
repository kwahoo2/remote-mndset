// SPDX-License-Identifier: BSL-1.0
/*!
 * @author Adrian Przekwas <adrian.v.przekwas@gmail.com>
 */


#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "structs.h"

#include <SDL3/SDL.h>

struct MovementModifier
{
    float walk; // forward - backward
    float sidestep; // left - right
    float altitude; // up - down
    float pitch;
    float yaw;
    float roll;
};

class Movement {
private:
    void checkKeyDown(SDL_Keycode key);
    void checkKeyUp(SDL_Keycode key);

    MovementModifier mov_mod{};
    float ang_vel, lin_vel, mouse_sens;
    float r_rate_mod;
    Uint64 old_ticks;
    float gamepad_axis_sens;
    float gamepad_dead_zone;
public:
    Movement();
    ~Movement();
    void updateConfigValues(float ang_v, float lin_v, float mouse_s, float g_axis_sens, float g_dead_zone);
    void passKeyboardEvent(SDL_Event& event);
    void passMouseRelativePos(float x, float y);
    void passGamepadState(SDL_Gamepad& gamepad);
    void updateTicks(Uint64 ticks);
    void updatePose(xrt_pose& pose);
    void updateVelocity(const xrt_pose& pose, const xrt_pose& old_pose, xrt_vec3& lin_vel, xrt_vec3& ang_vel);
};

#endif // MOVEMENT_H
