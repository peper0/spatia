#pragma once

#include "spatia/algebra/matrix_utils.hpp"
#include "spatia/geometry/angle.hpp"

namespace spatia {

/// The angle of `From` in `To` about their shared X axis, when interpreted as a `From` -> `To` rotation.
/// Positive angles follow the right-hand rule around the positive X axis in `To` (Y towards Z).
Matrix<3, 3> rotation_x(Angle angle_of_from_in_to);

/// The angle of `From` in `To` about their shared Y axis, when interpreted as a `From` -> `To` rotation.
/// Positive angles follow the right-hand rule around the positive Y axis in `To` (Z towards X).
Matrix<3, 3> rotation_y(Angle angle_of_from_in_to);

/// The angle of `From` in `To` about their shared Z axis, when interpreted as a `From` -> `To` rotation.
/// Positive angles follow the right-hand rule around the positive Z axis in `To` (X towards Y).
Matrix<3, 3> rotation_z(Angle angle_of_from_in_to);

// Implementation ======================================================================================================

inline Matrix<3, 3> rotation_x(Angle angle_of_from_in_to) { return rotation_x(angle_of_from_in_to.to_radians()); }

inline Matrix<3, 3> rotation_y(Angle angle_of_from_in_to) { return rotation_y(angle_of_from_in_to.to_radians()); }

inline Matrix<3, 3> rotation_z(Angle angle_of_from_in_to) { return rotation_z(angle_of_from_in_to.to_radians()); }

}  // namespace spatia
