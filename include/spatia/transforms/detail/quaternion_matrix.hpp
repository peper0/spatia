#pragma once

#include <algorithm>
#include <cmath>

#include "spatia/algebra/matrix.hpp"
#include "spatia/algebra/quaternion.hpp"

namespace spatia {
namespace detail {

Matrix<3, 3> quaternion_rotation_matrix(const Quaternion& quaternion);
Quaternion rotation_matrix_quaternion(const Matrix<3, 3>& rotation);
Quaternion canonicalized_rotation_quaternion(const Quaternion& quaternion);

inline Matrix<3, 3> quaternion_rotation_matrix(const Quaternion& quaternion) {
    const auto q = normalized(quaternion);
    const Scalar two = Scalar{2};
    return Matrix<3, 3>{
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

inline Quaternion rotation_matrix_quaternion(const Matrix<3, 3>& rotation) {
    Quaternion result;
    const Scalar trace = rotation(0, 0) + rotation(1, 1) + rotation(2, 2);

    if (trace > Scalar{}) {
        const Scalar scale = Scalar{2} * std::sqrt(std::max(Scalar{}, trace + Scalar{1}));
        result = {scale / Scalar{4}, (rotation(2, 1) - rotation(1, 2)) / scale,
                  (rotation(0, 2) - rotation(2, 0)) / scale, (rotation(1, 0) - rotation(0, 1)) / scale};
    } else if (rotation(0, 0) > rotation(1, 1) && rotation(0, 0) > rotation(2, 2)) {
        const Scalar scale =
            Scalar{2} * std::sqrt(std::max(Scalar{}, Scalar{1} + rotation(0, 0) - rotation(1, 1) - rotation(2, 2)));
        result = {(rotation(2, 1) - rotation(1, 2)) / scale, scale / Scalar{4},
                  (rotation(0, 1) + rotation(1, 0)) / scale, (rotation(0, 2) + rotation(2, 0)) / scale};
    } else if (rotation(1, 1) > rotation(2, 2)) {
        const Scalar scale =
            Scalar{2} * std::sqrt(std::max(Scalar{}, Scalar{1} - rotation(0, 0) + rotation(1, 1) - rotation(2, 2)));
        result = {(rotation(0, 2) - rotation(2, 0)) / scale, (rotation(0, 1) + rotation(1, 0)) / scale,
                  scale / Scalar{4}, (rotation(1, 2) + rotation(2, 1)) / scale};
    } else {
        const Scalar scale =
            Scalar{2} * std::sqrt(std::max(Scalar{}, Scalar{1} - rotation(0, 0) - rotation(1, 1) + rotation(2, 2)));
        result = {(rotation(1, 0) - rotation(0, 1)) / scale, (rotation(0, 2) + rotation(2, 0)) / scale,
                  (rotation(1, 2) + rotation(2, 1)) / scale, scale / Scalar{4}};
    }

    return canonicalized_rotation_quaternion(normalized(result));
}

inline Quaternion canonicalized_rotation_quaternion(const Quaternion& quaternion) {
    const bool negate =
        quaternion.w < Scalar{} || (quaternion.w == Scalar{} && quaternion.x < Scalar{}) ||
        (quaternion.w == Scalar{} && quaternion.x == Scalar{} && quaternion.y < Scalar{}) ||
        (quaternion.w == Scalar{} && quaternion.x == Scalar{} && quaternion.y == Scalar{} && quaternion.z < Scalar{});
    return negate ? -quaternion : quaternion;
}

}  // namespace detail
}  // namespace spatia
