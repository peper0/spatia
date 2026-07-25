#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

#include "geometry2/math/matrix.hpp"
#include "geometry2/math/quaternion.hpp"
#include "geometry2/math/roll_pitch_yaw.hpp"

namespace geometry2::math {

// All matrices in this header describe active, right-handed rotations acting
// on column vectors.
template <std::floating_point Scalar>
SquareMatrix<3, Scalar> rotation_x(Radians<Scalar> angle);

template <std::floating_point Scalar>
SquareMatrix<3, Scalar> rotation_y(Radians<Scalar> angle);

template <std::floating_point Scalar>
SquareMatrix<3, Scalar> rotation_z(Radians<Scalar> angle);

template <std::floating_point Scalar>
SquareMatrix<3, Scalar> to_rotation_matrix(
    const Quaternion<Scalar>& quaternion);

template <std::floating_point Scalar>
SquareMatrix<3, Scalar> to_rotation_matrix(const RollPitchYaw<Scalar>& angles);

// The matrix is expected to be a proper rotation matrix. The returned unit
// quaternion is sign-canonicalized because q and -q represent the same
// rotation: the first non-zero component in (w, x, y, z) is positive.
template <std::floating_point Scalar>
Quaternion<Scalar> to_quaternion(const SquareMatrix<3, Scalar>& rotation);

template <std::floating_point Scalar>
Quaternion<Scalar> to_quaternion(const RollPitchYaw<Scalar>& angles);

// The returned angles use pitch in [-pi/2, pi/2] and roll/yaw in [-pi, pi].
// At gimbal lock (pitch = +/-pi/2), roll is chosen as zero and yaw contains
// the remaining observable rotation.
template <std::floating_point Scalar>
RollPitchYaw<Scalar> to_roll_pitch_yaw(const SquareMatrix<3, Scalar>& rotation);

template <std::floating_point Scalar>
RollPitchYaw<Scalar> to_roll_pitch_yaw(const Quaternion<Scalar>& quaternion);

namespace detail {

template <std::floating_point Scalar>
Quaternion<Scalar> canonicalized_rotation_quaternion(
    const Quaternion<Scalar>& quaternion);

}  // namespace detail

// Implementation

template <std::floating_point Scalar>
SquareMatrix<3, Scalar> rotation_x(Radians<Scalar> angle) {
    const Scalar radians = angle.value();
    const Scalar cosine = std::cos(radians);
    const Scalar sine = std::sin(radians);
    return SquareMatrix<3, Scalar>{Scalar{1}, Scalar{0}, Scalar{0},
                                   Scalar{0}, cosine,    -sine,
                                   Scalar{0}, sine,      cosine};
}

template <std::floating_point Scalar>
SquareMatrix<3, Scalar> rotation_y(Radians<Scalar> angle) {
    const Scalar radians = angle.value();
    const Scalar cosine = std::cos(radians);
    const Scalar sine = std::sin(radians);
    return SquareMatrix<3, Scalar>{cosine,    Scalar{0}, sine,
                                   Scalar{0}, Scalar{1}, Scalar{0},
                                   -sine,     Scalar{0}, cosine};
}

template <std::floating_point Scalar>
SquareMatrix<3, Scalar> rotation_z(Radians<Scalar> angle) {
    const Scalar radians = angle.value();
    const Scalar cosine = std::cos(radians);
    const Scalar sine = std::sin(radians);
    return SquareMatrix<3, Scalar>{cosine,    -sine,     Scalar{0},
                                   sine,      cosine,    Scalar{0},
                                   Scalar{0}, Scalar{0}, Scalar{1}};
}

template <std::floating_point Scalar>
SquareMatrix<3, Scalar> to_rotation_matrix(
    const Quaternion<Scalar>& quaternion) {
    const auto q = normalized(quaternion);
    const Scalar two = Scalar{2};

    return SquareMatrix<3, Scalar>{
        Scalar{1} - two * (q.y * q.y + q.z * q.z),
        two * (q.x * q.y - q.w * q.z),
        two * (q.x * q.z + q.w * q.y),
        two * (q.x * q.y + q.w * q.z),
        Scalar{1} - two * (q.x * q.x + q.z * q.z),
        two * (q.y * q.z - q.w * q.x),
        two * (q.x * q.z - q.w * q.y),
        two * (q.y * q.z + q.w * q.x),
        Scalar{1} - two * (q.x * q.x + q.y * q.y),
    };
}

template <std::floating_point Scalar>
SquareMatrix<3, Scalar> to_rotation_matrix(const RollPitchYaw<Scalar>& angles) {
    return rotation_z(angles.yaw) * rotation_y(angles.pitch) *
           rotation_x(angles.roll);
}

template <std::floating_point Scalar>
Quaternion<Scalar> to_quaternion(const SquareMatrix<3, Scalar>& rotation) {
    Quaternion<Scalar> result;
    const Scalar trace = rotation(0, 0) + rotation(1, 1) + rotation(2, 2);

    if (trace > Scalar{}) {
        const Scalar scale =
            Scalar{2} * std::sqrt(std::max(Scalar{}, trace + Scalar{1}));
        result = {scale / Scalar{4}, (rotation(2, 1) - rotation(1, 2)) / scale,
                  (rotation(0, 2) - rotation(2, 0)) / scale,
                  (rotation(1, 0) - rotation(0, 1)) / scale};
    } else if (rotation(0, 0) > rotation(1, 1) &&
               rotation(0, 0) > rotation(2, 2)) {
        const Scalar scale =
            Scalar{2} *
            std::sqrt(std::max(Scalar{}, Scalar{1} + rotation(0, 0) -
                                             rotation(1, 1) - rotation(2, 2)));
        result = {(rotation(2, 1) - rotation(1, 2)) / scale, scale / Scalar{4},
                  (rotation(0, 1) + rotation(1, 0)) / scale,
                  (rotation(0, 2) + rotation(2, 0)) / scale};
    } else if (rotation(1, 1) > rotation(2, 2)) {
        const Scalar scale =
            Scalar{2} *
            std::sqrt(std::max(Scalar{}, Scalar{1} - rotation(0, 0) +
                                             rotation(1, 1) - rotation(2, 2)));
        result = {(rotation(0, 2) - rotation(2, 0)) / scale,
                  (rotation(0, 1) + rotation(1, 0)) / scale, scale / Scalar{4},
                  (rotation(1, 2) + rotation(2, 1)) / scale};
    } else {
        const Scalar scale =
            Scalar{2} *
            std::sqrt(std::max(Scalar{}, Scalar{1} - rotation(0, 0) -
                                             rotation(1, 1) + rotation(2, 2)));
        result = {(rotation(1, 0) - rotation(0, 1)) / scale,
                  (rotation(0, 2) + rotation(2, 0)) / scale,
                  (rotation(1, 2) + rotation(2, 1)) / scale, scale / Scalar{4}};
    }

    return detail::canonicalized_rotation_quaternion(normalized(result));
}

template <std::floating_point Scalar>
Quaternion<Scalar> to_quaternion(const RollPitchYaw<Scalar>& angles) {
    const Scalar half_roll = angles.roll.value() / Scalar{2};
    const Scalar half_pitch = angles.pitch.value() / Scalar{2};
    const Scalar half_yaw = angles.yaw.value() / Scalar{2};
    const Scalar cr = std::cos(half_roll);
    const Scalar sr = std::sin(half_roll);
    const Scalar cp = std::cos(half_pitch);
    const Scalar sp = std::sin(half_pitch);
    const Scalar cy = std::cos(half_yaw);
    const Scalar sy = std::sin(half_yaw);

    return detail::canonicalized_rotation_quaternion(
        normalized(Quaternion<Scalar>{
            cr * cp * cy + sr * sp * sy, sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy, cr * cp * sy - sr * sp * cy}));
}

template <std::floating_point Scalar>
RollPitchYaw<Scalar> to_roll_pitch_yaw(
    const SquareMatrix<3, Scalar>& rotation) {
    const Scalar horizontal = std::hypot(rotation(0, 0), rotation(1, 0));
    const Scalar gimbal_lock_threshold =
        std::sqrt(std::numeric_limits<Scalar>::epsilon());

    if (horizontal > gimbal_lock_threshold) {
        return {Radians<Scalar>{std::atan2(rotation(2, 1), rotation(2, 2))},
                Radians<Scalar>{std::atan2(-rotation(2, 0), horizontal)},
                Radians<Scalar>{std::atan2(rotation(1, 0), rotation(0, 0))}};
    }

    return {Radians<Scalar>{},
            Radians<Scalar>{std::copysign(
                std::numbers::pi_v<Scalar> / Scalar{2}, -rotation(2, 0))},
            Radians<Scalar>{std::atan2(-rotation(0, 1), rotation(1, 1))}};
}

template <std::floating_point Scalar>
RollPitchYaw<Scalar> to_roll_pitch_yaw(const Quaternion<Scalar>& quaternion) {
    return to_roll_pitch_yaw(to_rotation_matrix(quaternion));
}

namespace detail {

template <std::floating_point Scalar>
Quaternion<Scalar> canonicalized_rotation_quaternion(
    const Quaternion<Scalar>& quaternion) {
    const bool negate = quaternion.w < Scalar{} ||
                        (quaternion.w == Scalar{} && quaternion.x < Scalar{}) ||
                        (quaternion.w == Scalar{} && quaternion.x == Scalar{} &&
                         quaternion.y < Scalar{}) ||
                        (quaternion.w == Scalar{} && quaternion.x == Scalar{} &&
                         quaternion.y == Scalar{} && quaternion.z < Scalar{});
    return negate ? -quaternion : quaternion;
}

}  // namespace detail

}  // namespace geometry2::math
