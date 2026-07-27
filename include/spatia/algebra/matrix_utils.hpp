#pragma once

#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>

#include "spatia/algebra/matrix.hpp"
#include "spatia/config.hpp"

namespace spatia {

/// Inverse of a square matrix of any dimension, by Gauss-Jordan elimination
/// with partial pivoting. Dimensions 2 and 3 have closed-form overloads in
/// `matrix.hpp`, which are more specialized and win.
template <std::size_t Dimension>
SquareMatrix<Dimension> inverse(const SquareMatrix<Dimension>& matrix);

/// Rotation about the X axis by the given angle in radians.
Matrix<3, 3> rotation_x(Scalar radians);

/// Rotation about the Y axis by the given angle in radians.
Matrix<3, 3> rotation_y(Scalar radians);

/// Rotation about the Z axis by the given angle in radians.
Matrix<3, 3> rotation_z(Scalar radians);

// Implementation ======================================================================================================

template <std::size_t Dimension>
SquareMatrix<Dimension> inverse(const SquareMatrix<Dimension>& matrix) {
    SquareMatrix<Dimension> working = matrix;
    SquareMatrix<Dimension> result = identity_matrix<Dimension>();

    for (std::size_t column = 0; column < Dimension; ++column) {
        std::size_t pivot = column;
        for (std::size_t row = column + 1; row < Dimension; ++row) {
            if (std::abs(working(row, column)) > std::abs(working(pivot, column))) {
                pivot = row;
            }
        }
        if (std::abs(working(pivot, column)) <= std::numeric_limits<Scalar>::epsilon()) {
            throw std::invalid_argument("Cannot invert a singular matrix");
        }
        if (pivot != column) {
            for (std::size_t k = 0; k < Dimension; ++k) {
                const Scalar swapped_working = working(column, k);
                working(column, k) = working(pivot, k);
                working(pivot, k) = swapped_working;
                const Scalar swapped_result = result(column, k);
                result(column, k) = result(pivot, k);
                result(pivot, k) = swapped_result;
            }
        }

        const Scalar scale = working(column, column);
        for (std::size_t k = 0; k < Dimension; ++k) {
            working(column, k) /= scale;
            result(column, k) /= scale;
        }

        for (std::size_t row = 0; row < Dimension; ++row) {
            if (row == column) {
                continue;
            }
            const Scalar factor = working(row, column);
            if (factor == Scalar{}) {
                continue;
            }
            for (std::size_t k = 0; k < Dimension; ++k) {
                working(row, k) -= factor * working(column, k);
                result(row, k) -= factor * result(column, k);
            }
        }
    }

    return result;
}

inline Matrix<3, 3> rotation_x(Scalar radians) {
    const Scalar cosine = std::cos(radians);
    const Scalar sine = std::sin(radians);
    return Matrix<3, 3>{Scalar{1}, Scalar{0}, Scalar{0}, Scalar{0}, cosine, -sine, Scalar{0}, sine, cosine};
}

inline Matrix<3, 3> rotation_y(Scalar radians) {
    const Scalar cosine = std::cos(radians);
    const Scalar sine = std::sin(radians);
    return Matrix<3, 3>{cosine, Scalar{0}, sine, Scalar{0}, Scalar{1}, Scalar{0}, -sine, Scalar{0}, cosine};
}

inline Matrix<3, 3> rotation_z(Scalar radians) {
    const Scalar cosine = std::cos(radians);
    const Scalar sine = std::sin(radians);
    return Matrix<3, 3>{cosine, -sine, Scalar{0}, sine, cosine, Scalar{0}, Scalar{0}, Scalar{0}, Scalar{1}};
}

}  // namespace spatia
