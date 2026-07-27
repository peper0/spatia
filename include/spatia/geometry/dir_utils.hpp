#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

#include "spatia/geometry/angle.hpp"
#include "spatia/geometry/dir.hpp"
#include "spatia/geometry/euler.hpp"

namespace spatia {

template <class System>
Dir<System> reversed(const Dir<System>& direction);

template <class System>
constexpr Scalar dot(const Dir<System>& left, const Dir<System>& right);

template <class System>
Angle angle_between(const Dir<System>& left, const Dir<System>& right);

template <class System>
Dir<System> to_dir(const EulerZY<System>& angles);

template <class System>
EulerZY<System> to_euler_zy(const Dir<System>& direction);

// Implementation ======================================================================================================

template <class System>
Dir<System> reversed(const Dir<System>& direction) {
    return Dir<System>::from_vector(-to_vector(direction));
}

template <class System>
constexpr Scalar dot(const Dir<System>& left, const Dir<System>& right) {
    return dot(to_vector(left), to_vector(right));
}

template <class System>
Angle angle_between(const Dir<System>& left, const Dir<System>& right) {
    const Scalar cosine = std::clamp(dot(left, right), Scalar{-1}, Scalar{1});
    return Angle::from_radians(std::acos(cosine));
}

template <class System>
Dir<System> to_dir(const EulerZY<System>& angles) {
    const Scalar z = angles.z().to_radians();
    const Scalar y = angles.y().to_radians();
    const Scalar cos_y = std::cos(y);
    return Dir<System>::from_vector(Vector<System>{std::cos(z) * cos_y, std::sin(z) * cos_y, -std::sin(y)});
}

template <class System>
EulerZY<System> to_euler_zy(const Dir<System>& direction) {
    const Vector<System>& unit = to_vector(direction);
    const Scalar horizontal = std::hypot(unit.dx(), unit.dy());
    const Scalar z = horizontal > std::numeric_limits<Scalar>::epsilon() ? std::atan2(unit.dy(), unit.dx()) : Scalar{};
    const Scalar y = std::atan2(-unit.dz(), horizontal);
    return {Angle::from_radians(z), Angle::from_radians(y)};
}

}  // namespace spatia
