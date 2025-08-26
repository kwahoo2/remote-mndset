// SPDX-License-Identifier: BSL-1.0
/*!
 * @author Adrian Przekwas <adrian.v.przekwas@gmail.com>
 */

#include "math_helper.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"

// angles in radians
struct xrt_quat quatFromYXZ(float yaw, float pitch, float roll) {
    glm::mat4 m = glm::eulerAngleYXZ(yaw, pitch, roll);
    glm::quat q_glm = glm::quat_cast(m);

    xrt_quat q {q_glm.x, q_glm.y, q_glm.z, q_glm.w};
    return q;
}

std::tuple<float, float, float> quatToYXZ(const xrt_quat& q) {
    glm::quat q_glm = glm::quat(q.w, q.x, q.y, q.z);
    glm::mat4 m = glm::mat4_cast(q_glm);

    float yaw, pitch, roll;
    glm::extractEulerAngleYXZ(m, yaw, pitch, roll);

    return {yaw, pitch, roll};
}

// result is a vector rotated by a quaternion
struct xrt_vec3 quatMultVec(const xrt_quat& q, const xrt_vec3& v) {
    const float qw = q.w, qx = q.x, qy = q.y, qz = q.z;
    const float vx = v.x, vy = v.y, vz = v.z;

    // q * v
    const float temp_w = -qx * vx - qy * vy - qz * vz;
    const float temp_x =  qw * vx + qy * vz - qz * vy;
    const float temp_y =  qw * vy + qz * vx - qx * vz;
    const float temp_z =  qw * vz + qx * vy - qy * vx;

    // (q * v) * q^-1
    const float result_x = temp_w * -qx + temp_x * qw + temp_y * -qz - temp_z * -qy;
    const float result_y = temp_w * -qy - temp_x * -qz + temp_y * qw + temp_z * -qx;
    const float result_z = temp_w * -qz + temp_x * -qy - temp_y * -qx + temp_z * qw;

    struct xrt_vec3 result = {result_x, result_y, result_z};
    return result;
}

struct xrt_pose poseMult(const xrt_pose& a, const xrt_pose& b) {
    xrt_pose result;
    result.orientation = a.orientation * b.orientation;
    result.position = a.position + quatMultVec(a.orientation, b.position);
    return result;
}
