// SPDX-License-Identifier: BSL-1.0
/*!
 * @author Adrian Przekwas <adrian.v.przekwas@gmail.com>
 */


#ifndef MATH_HELPER_H
#define MATH_HELPER_H

#include "structs.h"

#include <tuple>

struct xrt_quat quatFromYXZ(float yaw, float pitch, float roll);
std::tuple<float, float, float>  quatToYXZ(const xrt_quat& q);
struct xrt_vec3 quatMultVec(const xrt_quat& q, const xrt_vec3& v);
struct xrt_pose poseMult(const xrt_pose& a, const xrt_pose& b);
xrt_vec3 calculateAngularVel(const xrt_quat& q1, const xrt_quat& q2, float dt);

#endif // MATH_HELPER_H
