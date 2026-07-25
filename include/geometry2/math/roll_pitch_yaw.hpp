#pragma once

#include <concepts>

#include "geometry2/math/radians.hpp"

namespace geometry2::math {

// Roll-pitch-yaw convention:
// - all angles use the strong Radians type,
// - rotations are active, right-handed and act on column vectors,
// - R = Rz(yaw) * Ry(pitch) * Rx(roll),
// - equivalently: extrinsic rotations about fixed X, Y, Z axes, or intrinsic
//   yaw-pitch-roll rotations about moving Z, Y, X axes.
template <std::floating_point Scalar = double>
struct RollPitchYaw {
    using scalar_type = Scalar;

    Radians<Scalar> roll{};
    Radians<Scalar> pitch{};
    Radians<Scalar> yaw{};

    constexpr RollPitchYaw();
    constexpr RollPitchYaw(Radians<Scalar> roll_angle,
                           Radians<Scalar> pitch_angle,
                           Radians<Scalar> yaw_angle);

    constexpr bool operator==(const RollPitchYaw&) const;
};

// Implementation

template <std::floating_point Scalar>
constexpr RollPitchYaw<Scalar>::RollPitchYaw() = default;

template <std::floating_point Scalar>
constexpr RollPitchYaw<Scalar>::RollPitchYaw(Radians<Scalar> roll_angle,
                                             Radians<Scalar> pitch_angle,
                                             Radians<Scalar> yaw_angle)
    : roll(roll_angle), pitch(pitch_angle), yaw(yaw_angle) {}

template <std::floating_point Scalar>
constexpr bool RollPitchYaw<Scalar>::operator==(const RollPitchYaw&) const =
    default;

}  // namespace geometry2::math
