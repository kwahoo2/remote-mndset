// Structs based on: https://gitlab.freedesktop.org/monado/monado
// SPDX-License-Identifier: BSL-1.0
/*!
 * @author Adrian Przekwas <adrian.v.przekwas@gmail.com>
 */

#ifndef STRUCTS_H
#define STRUCTS_H

#include <cstdint> // for fixed size integer types
#include <string>

#define	MONADO_PORT	4242
#define R_HEADER_VALUE (*(uint64_t *)"mndrmt3\0") // used by Monado since e4931a46bd0a161a15a1a601f927d1dec30d23ce

struct xrt_vec1 {
    float x;
};
struct xrt_vec2 {
    float x;
    float y;
};
struct xrt_vec3 {
    float x;
    float y;
    float z;
    xrt_vec3 operator+(const xrt_vec3& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }
};

struct xrt_quat {
    float x;
    float y;
    float z;
    float w;

    xrt_quat operator*(const xrt_quat& other) const {
        return {
            w * other.x + x * other.w + y * other.z - z * other.y,  // x
            w * other.y - x * other.z + y * other.w + z * other.x,  // y
            w * other.z + x * other.y - y * other.x + z * other.w,  // z
            w * other.w - x * other.x - y * other.y - z * other.z   // w
        };
    }
};

struct xrt_pose {
    struct xrt_quat orientation;
    struct xrt_vec3 position;
};

struct r_remote_controller_data {
    struct xrt_pose pose;
    struct xrt_vec3 linear_velocity;
    struct xrt_vec3 angular_velocity;
    float hand_curl[5];
    struct xrt_vec1 trigger_value;
    struct xrt_vec1 squeeze_value;
    struct xrt_vec1 squeeze_force;
    struct xrt_vec2 thumbstick;
    struct xrt_vec1 trackpad_force;
    struct xrt_vec2 trackpad;
    bool hand_tracking_active;
    bool active;
    bool system_click;
    bool system_touch;
    bool a_click;
    bool a_touch;
    bool b_click;
    bool b_touch;
    bool trigger_click;
    bool trigger_touch;
    bool thumbstick_click;
    bool thumbstick_touch;
    bool trackpad_touch;
    bool _pad0;
    bool _pad1;
    bool _pad2;
    // active(2) + bools(11) + pad(3) = 16
};

struct xrt_fov {
    float angle_left;
    float angle_right;
    float angle_up;
    float angle_down;
};

struct r_head_data {
    struct {
        //! The field of view values of this view.
        struct xrt_fov fov;
        //! The pose of this view relative to @ref r_head_data::center.
        struct xrt_pose pose;
        //! Padded to fov(16) + pose(16 + 12) + 4 = 48
        std::uint32_t _pad;
    } views[2];
    //! The center of the head, in OpenXR terms the view space.
    struct xrt_pose center;
    //! Is the per view data valid and should be used?
    bool per_view_data_valid;
    //! pose(16 + 12) bool(1) + pad(3) = 32.
    bool _pad0, _pad1, _pad2;
};

struct r_remote_data {
    uint64_t header;
    struct r_head_data head;
    struct r_remote_controller_data left, right;
};

// what part of VR system should be moved with keyboard, mouse or gamepad
enum InputConsumer {
    hmd = 0,
    left_controller = 1,
    right_controller = 2
};

struct Config {
    float hmd_lin_vel;
    float hmd_ang_vel;
    float controller_lin_vel;
    float controller_ang_vel;
    float mouse_sens;
    float gamepad_axis_sens;
    float gamepad_dead_zone;
    std::string server_ip;
};

// state of an ImGui window
struct WindowState {
    bool connect_button_clicked = false;
    bool grab_button_clicked = false;
    InputConsumer iCons = hmd;
    bool has_gamepad = false;
    std::string gamepad_name = "";
    int batt = -1;
    Config config {};
};

static const float gamepad_axis_range = 32768.0f;
static const float gamepad_click_threshold = 0.7f;

#endif // STRUCTS_H
