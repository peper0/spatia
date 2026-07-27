#pragma once

#include <cmath>
#include <limits>

#include "spatia/geometry/angle.hpp"
#include "spatia/geometry/line.hpp"

namespace spatia {

/// True when the two lines are parallel (including anti-parallel) within the
/// given angular tolerance.
template <class System>
bool are_parallel(const Line<System>& left, const Line<System>& right, Angle tolerance = Angle::from_degrees(1e-6));

/// Minimal distance between the two lines.
template <class System>
Scalar distance(const Line<System>& left, const Line<System>& right);

// Implementation ======================================================================================================

template <class System>
bool are_parallel(const Line<System>& left, const Line<System>& right, Angle tolerance) {
    // The angle is measured through the perpendicular component of one
    // direction with respect to the other. Its norm equals sin(angle) and,
    // unlike a cosine comparison, stays accurate for the tiny default
    // tolerance. The perpendicular component is invariant under flipping a
    // direction, so anti-parallel lines count as parallel.
    const auto left_direction = to_vector(dir_of(left));
    const auto right_direction = to_vector(dir_of(right));
    const auto perpendicular = left_direction - dot(left_direction, right_direction) * right_direction;
    return norm(perpendicular) <= std::sin(tolerance.to_radians());
}

template <class System>
Scalar distance(const Line<System>& left, const Line<System>& right) {
    // Minimize |left(t) - right(s)| over the two line parameters. With unit
    // directions the normal equations collapse to a 2x2 system whose
    // determinant is 1 - dot(d_left, d_right)^2; a vanishing determinant
    // means the lines are parallel.
    const auto left_direction = to_vector(dir_of(left));
    const auto right_direction = to_vector(dir_of(right));
    const auto offset = closest_point_to_origin(left) - closest_point_to_origin(right);

    const Scalar alignment = dot(left_direction, right_direction);
    const Scalar along_left = dot(left_direction, offset);
    const Scalar along_right = dot(right_direction, offset);
    const Scalar determinant = Scalar{1} - alignment * alignment;

    if (determinant <= std::numeric_limits<Scalar>::epsilon()) {
        const auto perpendicular = offset - along_left * left_direction;
        return norm(perpendicular);
    }

    const Scalar left_parameter = (alignment * along_right - along_left) / determinant;
    const Scalar right_parameter = (along_right - alignment * along_left) / determinant;
    return norm(offset + left_parameter * left_direction - right_parameter * right_direction);
}

}  // namespace spatia
