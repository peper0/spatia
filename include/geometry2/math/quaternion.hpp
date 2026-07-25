#pragma once

#include <cmath>
#include <concepts>
#include <limits>
#include <stdexcept>

namespace geometry2::math {

// Quaternion convention:
// - component and constructor order is scalar-first (w, x, y, z),
// - multiplication is the Hamilton product,
// - a rotation quaternion is interpreted as an active, right-handed rotation:
//   v_rotated = q * (0, v) * conjugated(q),
// - therefore q_left * q_right applies q_right first and q_left second.
// The default value is the mathematical zero quaternion, which is not a valid
// rotation; the identity rotation is Quaternion{1, 0, 0, 0}.
template <std::floating_point Scalar = double>
struct Quaternion {
    using scalar_type = Scalar;

    Scalar w{};
    Scalar x{};
    Scalar y{};
    Scalar z{};

    constexpr Quaternion();
    constexpr Quaternion(Scalar w_value, Scalar x_value, Scalar y_value,
                         Scalar z_value);

    constexpr bool operator==(const Quaternion&) const;
};

template <std::floating_point Scalar>
constexpr Quaternion<Scalar> operator-(const Quaternion<Scalar>& quaternion);

template <std::floating_point Scalar>
constexpr Quaternion<Scalar> operator*(const Quaternion<Scalar>& left,
                                       const Quaternion<Scalar>& right);

template <std::floating_point Scalar>
constexpr Quaternion<Scalar> conjugated(const Quaternion<Scalar>& quaternion);

template <std::floating_point Scalar>
constexpr Scalar squared_norm(const Quaternion<Scalar>& quaternion);

template <std::floating_point Scalar>
Scalar norm(const Quaternion<Scalar>& quaternion);

template <std::floating_point Scalar>
Quaternion<Scalar> normalized(const Quaternion<Scalar>& quaternion);

// Implementation

template <std::floating_point Scalar>
constexpr Quaternion<Scalar>::Quaternion() = default;

template <std::floating_point Scalar>
constexpr Quaternion<Scalar>::Quaternion(Scalar w_value, Scalar x_value,
                                         Scalar y_value, Scalar z_value)
    : w(w_value), x(x_value), y(y_value), z(z_value) {}

template <std::floating_point Scalar>
constexpr bool Quaternion<Scalar>::operator==(const Quaternion&) const =
    default;

template <std::floating_point Scalar>
constexpr Quaternion<Scalar> operator-(const Quaternion<Scalar>& quaternion) {
    return {-quaternion.w, -quaternion.x, -quaternion.y, -quaternion.z};
}

template <std::floating_point Scalar>
constexpr Quaternion<Scalar> operator*(const Quaternion<Scalar>& left,
                                       const Quaternion<Scalar>& right) {
    return {
        left.w * right.w - left.x * right.x - left.y * right.y -
            left.z * right.z,
        left.w * right.x + left.x * right.w + left.y * right.z -
            left.z * right.y,
        left.w * right.y - left.x * right.z + left.y * right.w +
            left.z * right.x,
        left.w * right.z + left.x * right.y - left.y * right.x +
            left.z * right.w,
    };
}

template <std::floating_point Scalar>
constexpr Quaternion<Scalar> conjugated(const Quaternion<Scalar>& quaternion) {
    return {quaternion.w, -quaternion.x, -quaternion.y, -quaternion.z};
}

template <std::floating_point Scalar>
constexpr Scalar squared_norm(const Quaternion<Scalar>& quaternion) {
    return quaternion.w * quaternion.w + quaternion.x * quaternion.x +
           quaternion.y * quaternion.y + quaternion.z * quaternion.z;
}

template <std::floating_point Scalar>
Scalar norm(const Quaternion<Scalar>& quaternion) {
    return std::sqrt(squared_norm(quaternion));
}

template <std::floating_point Scalar>
Quaternion<Scalar> normalized(const Quaternion<Scalar>& quaternion) {
    const Scalar quaternion_norm = norm(quaternion);
    if (quaternion_norm <= std::numeric_limits<Scalar>::epsilon()) {
        throw std::invalid_argument("Cannot normalize a zero quaternion");
    }
    return {quaternion.w / quaternion_norm, quaternion.x / quaternion_norm,
            quaternion.y / quaternion_norm, quaternion.z / quaternion_norm};
}

}  // namespace geometry2::math
