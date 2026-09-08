#pragma once

#include <cmath>
#include <limits>
#include <numbers>

#include "spatia/algebra/matrix_utils.hpp"
#include "spatia/algebra/quaternion.hpp"
#include "spatia/geometry/euler.hpp"
#include "spatia/transforms/affine.hpp"
#include "spatia/transforms/detail/angle_matrix.hpp"
#include "spatia/transforms/detail/quaternion_matrix.hpp"
#include "spatia/transforms/perspective_projection.hpp"
#include "spatia/transforms/projective.hpp"
#include "spatia/transforms/rigid.hpp"
#include "spatia/transforms/rotation.hpp"
#include "spatia/transforms/translation.hpp"

namespace spatia {

template <class From, class To>
    requires(dimension_v<From> == 3 && dimension_v<To> == 3)
Rotation<From, To> to_rotation(const EulerZYX<From, To>& angles_of_from_in_to);

template <class From, class To>
Rotation<From, To> to_rotation(const Quaternion& quaternion);

template <class From, class To>
EulerZYX<From, To> to_euler_zyx(const Rotation<From, To>& transform);

template <class From, class To>
Quaternion to_quaternion(const Rotation<From, To>& transform);

template <class From, class To>
    requires(dimension_v<From> == 2 && dimension_v<To> == 2)
Rotation<From, To> to_rotation(Angle angle_of_from_in_to);

template <class From, class To>
    requires(dimension_v<From> == 2 && dimension_v<To> == 2)
Angle to_angle(const Rotation<From, To>& transform);

template <class From, class To>
constexpr const Matrix<dimension_v<From>, dimension_v<To>>& to_matrix(const Rotation<From, To>& transform);

Matrix<3, 3> to_matrix(const Quaternion& quaternion);

template <class From, class To>
    requires(dimension_v<From> == 3 && dimension_v<To> == 3)
Matrix<3, 3> to_matrix(const EulerZYX<From, To>& angles);

// Bidirectional conversions: expose the forward and reverse mappings in one
// object, preserving the transform kind.

template <class From, class To>
constexpr BiRotation<From, To> to_bidirectional(const Rotation<From, To>& transform);

template <class From, class To>
constexpr BiTranslation<From, To> to_bidirectional(const Translation<From, To>& transform);

template <class From, class To>
constexpr BiRigid<From, To> to_bidirectional(const Rigid<From, To>& transform);

/// Requires an invertible linear part; construction computes its inverse.
template <class From, class To>
BiAffine<From, To> to_bidirectional(const Affine<From, To>& transform);

/// The reverse perspective mapping recovers a direction in space.
template <class FromSpace, class ToPlane>
BiPerspectiveProjection<FromSpace, ToPlane> to_bidirectional(const PerspectiveProjection<FromSpace, ToPlane>& transform);

template <class FromPlane, class ToSpace>
BiPerspectiveProjection<ToSpace, FromPlane> to_bidirectional(const PerspectiveUnprojection<FromPlane, ToSpace>& transform);

/// The default tolerance of the narrowing conversions below, expressed as an
/// absolute error on matrix entries and translation coordinates.
inline constexpr Scalar default_narrowing_tolerance = Scalar{1e-9};

// Widening conversions: every transform kind is a special case of the more
// general ones, so these always succeed.

template <class From, class To>
constexpr Affine<From, To> to_affine(const Rotation<From, To>& transform);

template <class From, class To>
constexpr Affine<From, To> to_affine(const Translation<From, To>& transform);

template <class From, class To>
constexpr Affine<From, To> to_affine(const Rigid<From, To>& transform);

template <class From, class To>
constexpr Rigid<From, To> to_rigid(const Rotation<From, To>& transform);

template <class From, class To>
constexpr Rigid<From, To> to_rigid(const Translation<From, To>& transform);

template <class From, class To>
Projective<From, To> to_projective(const Affine<From, To>& transform);

template <class From, class To>
Projective<From, To> to_projective(const Rigid<From, To>& transform);

template <class From, class To>
Projective<From, To> to_projective(const Rotation<From, To>& transform);

template <class From, class To>
Projective<From, To> to_projective(const Translation<From, To>& transform);

template <class FromSpace, class ToPlane>
Projective<FromSpace, ToPlane> to_projective(const PerspectiveProjection<FromSpace, ToPlane>& transform);

// Narrowing conversions: the general transform must already satisfy the
// invariant of the narrower kind, up to `tolerance`, otherwise the conversion
// throws `std::invalid_argument`.

template <class From, class To>
Rigid<From, To> to_rigid(const Affine<From, To>& transform, Scalar tolerance = default_narrowing_tolerance);

template <class From, class To>
Rotation<From, To> to_rotation(const Affine<From, To>& transform, Scalar tolerance = default_narrowing_tolerance);  // AI: te Narrowing conversions powinny byc as_...

template <class From, class To>
Rotation<From, To> to_rotation(const Rigid<From, To>& transform, Scalar tolerance = default_narrowing_tolerance);

template <class From, class To>
Translation<From, To> to_translation(const Affine<From, To>& transform, Scalar tolerance = default_narrowing_tolerance);

template <class From, class To>
Translation<From, To> to_translation(const Rigid<From, To>& transform, Scalar tolerance = default_narrowing_tolerance);

// Implementation ======================================================================================================

template <class From, class To>
    requires(dimension_v<From> == 3 && dimension_v<To> == 3)
Rotation<From, To> to_rotation(const EulerZYX<From, To>& angles_of_from_in_to) {
    return Rotation<From, To>{rotation_z(angles_of_from_in_to.z()) * rotation_y(angles_of_from_in_to.y()) *
                              rotation_x(angles_of_from_in_to.x())};
}

template <class From, class To>
Rotation<From, To> to_rotation(const Quaternion& quaternion) {
    return Rotation<From, To>{detail::quaternion_rotation_matrix(quaternion)};
}

template <class From, class To>
EulerZYX<From, To> to_euler_zyx(const Rotation<From, To>& transform) {
    const auto& rotation = transform.to_matrix();
    const Scalar horizontal = std::hypot(rotation(0, 0), rotation(1, 0));
    const Scalar gimbal_lock_threshold = std::sqrt(std::numeric_limits<Scalar>::epsilon());

    if (horizontal > gimbal_lock_threshold) {
        return {Angle::from_radians(std::atan2(rotation(1, 0), rotation(0, 0))),
                Angle::from_radians(std::atan2(-rotation(2, 0), horizontal)),
                Angle::from_radians(std::atan2(rotation(2, 1), rotation(2, 2)))};
    }

    return {Angle::from_radians(std::atan2(-rotation(0, 1), rotation(1, 1))),
            Angle::from_radians(std::copysign(std::numbers::pi_v<Scalar> / Scalar{2}, -rotation(2, 0))), Angle{}};
}

template <class From, class To>
Quaternion to_quaternion(const Rotation<From, To>& transform) {
    return transform.to_quaternion();
}

template <class From, class To>
    requires(dimension_v<From> == 2 && dimension_v<To> == 2)
Rotation<From, To> to_rotation(Angle angle_of_from_in_to) {
    const Scalar cosine = std::cos(angle_of_from_in_to.to_radians());
    const Scalar sine = std::sin(angle_of_from_in_to.to_radians());
    return Rotation<From, To>{Matrix<2, 2>{cosine, -sine, sine, cosine}};
}

template <class From, class To>
    requires(dimension_v<From> == 2 && dimension_v<To> == 2)
Angle to_angle(const Rotation<From, To>& transform) {
    const auto& rotation = transform.to_matrix();
    return Angle::from_radians(std::atan2(rotation(1, 0), rotation(0, 0)));
}

template <class From, class To>
constexpr const Matrix<dimension_v<From>, dimension_v<To>>& to_matrix(const Rotation<From, To>& transform) {
    return transform.to_matrix();
}

inline Matrix<3, 3> to_matrix(const Quaternion& quaternion) { return detail::quaternion_rotation_matrix(quaternion); }

template <class From, class To>
    requires(dimension_v<From> == 3 && dimension_v<To> == 3)
Matrix<3, 3> to_matrix(const EulerZYX<From, To>& angles) {
    return to_rotation(angles).to_matrix();
}

template <class From, class To>
constexpr BiRotation<From, To> to_bidirectional(const Rotation<From, To>& transform) {
    return {transform};
}

template <class From, class To>
constexpr BiTranslation<From, To> to_bidirectional(const Translation<From, To>& transform) {
    return {transform};
}

template <class From, class To>
constexpr BiRigid<From, To> to_bidirectional(const Rigid<From, To>& transform) {
    return {transform};
}

template <class From, class To>
BiAffine<From, To> to_bidirectional(const Affine<From, To>& transform) {
    return {transform};
}

template <class FromSpace, class ToPlane>
BiPerspectiveProjection<FromSpace, ToPlane> to_bidirectional(const PerspectiveProjection<FromSpace, ToPlane>& transform) {
    return {transform.scale_x(), transform.scale_y(), transform.center_x(), transform.center_y()};
}

template <class FromPlane, class ToSpace>
BiPerspectiveProjection<ToSpace, FromPlane> to_bidirectional(const PerspectiveUnprojection<FromPlane, ToSpace>& transform) {
    return {transform.scale_x(), transform.scale_y(), transform.center_x(), transform.center_y()};
}

template <class From, class To>
constexpr Affine<From, To> to_affine(const Rotation<From, To>& transform) {
    return {transform.to_matrix(), Vector<To>{}};
}

template <class From, class To>
constexpr Affine<From, To> to_affine(const Translation<From, To>& transform) {
    return {identity_matrix<dimension_v<From>>(), transform.translation()};
}

template <class From, class To>
constexpr Affine<From, To> to_affine(const Rigid<From, To>& transform) {
    return {transform.rotation(), transform.from_origin_in_to() - Point<To>{}};
}

template <class From, class To>
constexpr Rigid<From, To> to_rigid(const Rotation<From, To>& transform) {
    return {transform.to_matrix(), Point<To>{}};
}

template <class From, class To>
constexpr Rigid<From, To> to_rigid(const Translation<From, To>& transform) {
    return {identity_matrix<dimension_v<From>>(), transform(Point<From>{})};
}

namespace detail {

// Packs a linear part and an offset into a homogeneous matrix.
template <std::size_t Dimension>
Matrix<Dimension + 1, Dimension + 1> homogeneous_matrix(const Matrix<Dimension, Dimension>& linear,
                                                        const Vec<Dimension>& offset) {
    Matrix<Dimension + 1, Dimension + 1> result;
    for (std::size_t row = 0; row < Dimension; ++row) {
        for (std::size_t column = 0; column < Dimension; ++column) {
            result(row, column) = linear(row, column);
        }
        result(row, Dimension) = offset[row];
    }
    result(Dimension, Dimension) = Scalar{1};
    return result;
}

}  // namespace detail

template <class From, class To>
Projective<From, To> to_projective(const Affine<From, To>& transform) {
    return Projective<From, To>{
        detail::homogeneous_matrix<dimension_v<From>>(transform.linear(), transform.translation().to_vec())};
}

template <class From, class To>
Projective<From, To> to_projective(const Rigid<From, To>& transform) {
    return to_projective(to_affine(transform));
}

template <class From, class To>
Projective<From, To> to_projective(const Rotation<From, To>& transform) {
    return to_projective(to_affine(transform));
}

template <class From, class To>
Projective<From, To> to_projective(const Translation<From, To>& transform) {
    return to_projective(to_affine(transform));
}

template <class FromSpace, class ToPlane>
Projective<FromSpace, ToPlane> to_projective(const PerspectiveProjection<FromSpace, ToPlane>& transform) {
    // Dividing by the third coordinate is exactly what the homogeneous
    // representation does for free, so the projection is linear here: the
    // weight row copies that coordinate.
    return Projective<FromSpace, ToPlane>{Matrix<3, 4>{transform.scale_x(), Scalar{}, transform.center_x(), Scalar{},
                                                       Scalar{}, transform.scale_y(), transform.center_y(), Scalar{},
                                                       Scalar{}, Scalar{}, Scalar{1}, Scalar{}}};
}

namespace detail {

template <std::size_t Dimension>
bool is_orthonormal(const Matrix<Dimension, Dimension>& matrix, Scalar tolerance) {
    const auto product = transposed(matrix) * matrix;
    const auto identity = identity_matrix<Dimension>();
    for (std::size_t row = 0; row < Dimension; ++row) {
        for (std::size_t column = 0; column < Dimension; ++column) {
            if (std::abs(product(row, column) - identity(row, column)) > tolerance) {
                return false;
            }
        }
    }
    return true;
}

template <std::size_t Dimension>
bool is_near_zero(const Vec<Dimension>& coordinates, Scalar tolerance) {
    for (std::size_t i = 0; i < Dimension; ++i) {
        if (std::abs(coordinates[i]) > tolerance) {
            return false;
        }
    }
    return true;
}

}  // namespace detail

template <class From, class To>
Rigid<From, To> to_rigid(const Affine<From, To>& transform, Scalar tolerance) {
    if (!detail::is_orthonormal(transform.linear(), tolerance)) {
        throw std::invalid_argument("Affine transform is not rigid: its linear part is not a rotation");
    }
    return {transform.linear(), Point<To>{} + transform.translation()};
}

template <class From, class To>
Rotation<From, To> to_rotation(const Affine<From, To>& transform, Scalar tolerance) {
    if (!detail::is_near_zero(transform.translation().to_vec(), tolerance)) {
        throw std::invalid_argument("Affine transform is not a rotation: it translates the origin");
    }
    return to_rotation(to_rigid(transform, tolerance), tolerance);
}

template <class From, class To>
Rotation<From, To> to_rotation(const Rigid<From, To>& transform, Scalar tolerance) {
    if (!detail::is_near_zero(transform.from_origin_in_to().to_vec(), tolerance)) {
        throw std::invalid_argument("Rigid transform is not a rotation: it translates the origin");
    }
    return Rotation<From, To>{transform.rotation()};
}

template <class From, class To>
Translation<From, To> to_translation(const Affine<From, To>& transform, Scalar tolerance) {
    return to_translation(to_rigid(transform, tolerance), tolerance);
}

template <class From, class To>
Translation<From, To> to_translation(const Rigid<From, To>& transform, Scalar tolerance) {
    const auto difference = transform.rotation();
    const auto identity = identity_matrix<dimension_v<From>>();
    for (std::size_t row = 0; row < dimension_v<From>; ++row) {
        for (std::size_t column = 0; column < dimension_v<From>; ++column) {
            if (std::abs(difference(row, column) - identity(row, column)) > tolerance) {
                throw std::invalid_argument("Rigid transform is not a translation: it rotates the axes");
            }
        }
    }
    return Translation<From, To>{transform.from_origin_in_to()};
}

}  // namespace spatia
