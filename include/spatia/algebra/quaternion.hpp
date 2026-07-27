#pragma once

#include <cmath>
#include <limits>
#include <stdexcept>

#include "spatia/config.hpp"

namespace spatia {

/// Algebraic quaternion without coordinate-system semantics.
///
/// Component and constructor order is scalar-first `(w, x, y, z)`.
/// Multiplication is the Hamilton product. A unit quaternion represents an
/// active, right-handed rotation of column vectors as `q * (0, v) * conj(q)`.
/// Thus `left * right` applies `right` first. `q` and `-q` encode the same
/// rotation; conversions supplied by spatia choose a canonical sign.
struct Quaternion {
    Scalar w{};
    Scalar x{};
    Scalar y{};
    Scalar z{};

    constexpr Quaternion() = default;
    constexpr Quaternion(Scalar w_value, Scalar x_value, Scalar y_value, Scalar z_value)
        : w(w_value), x(x_value), y(y_value), z(z_value) {}
    constexpr bool operator==(const Quaternion&) const = default;
};

constexpr Quaternion operator-(const Quaternion& quaternion);
constexpr Quaternion operator*(const Quaternion& left, const Quaternion& right);
constexpr Quaternion conjugated(const Quaternion& quaternion);
constexpr Scalar squared_norm(const Quaternion& quaternion);
Scalar norm(const Quaternion& quaternion);
Quaternion normalized(const Quaternion& quaternion);

// Implementation ======================================================================================================

constexpr Quaternion operator-(const Quaternion& quaternion) {
    return {-quaternion.w, -quaternion.x, -quaternion.y, -quaternion.z};
}

constexpr Quaternion operator*(const Quaternion& left, const Quaternion& right) {
    return {
        left.w * right.w - left.x * right.x - left.y * right.y - left.z * right.z,
        left.w * right.x + left.x * right.w + left.y * right.z - left.z * right.y,
        left.w * right.y - left.x * right.z + left.y * right.w + left.z * right.x,
        left.w * right.z + left.x * right.y - left.y * right.x + left.z * right.w,
    };
}

constexpr Quaternion conjugated(const Quaternion& quaternion) {
    return {quaternion.w, -quaternion.x, -quaternion.y, -quaternion.z};
}

constexpr Scalar squared_norm(const Quaternion& quaternion) {
    return quaternion.w * quaternion.w + quaternion.x * quaternion.x + quaternion.y * quaternion.y +
           quaternion.z * quaternion.z;
}

inline Scalar norm(const Quaternion& quaternion) { return std::sqrt(squared_norm(quaternion)); }

inline Quaternion normalized(const Quaternion& quaternion) {
    const Scalar quaternion_norm = norm(quaternion);
    if (quaternion_norm <= std::numeric_limits<Scalar>::epsilon()) {
        throw std::invalid_argument("Cannot normalize a zero quaternion");
    }
    return {quaternion.w / quaternion_norm, quaternion.x / quaternion_norm, quaternion.y / quaternion_norm,
            quaternion.z / quaternion_norm};
}

}  // namespace spatia
