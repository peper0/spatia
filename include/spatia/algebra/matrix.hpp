#pragma once

#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

#include "spatia/algebra/vec.hpp"
#include "spatia/config.hpp"

namespace spatia {

/// Algebraic matrix without coordinate-system semantics.
template <std::size_t Rows, std::size_t Columns>
class Matrix {
    static_assert(Rows > 0 && Columns > 0);

   public:
    static constexpr std::size_t row_count = Rows;
    static constexpr std::size_t column_count = Columns;

    constexpr Matrix() = default;

    template <class... Values>
        requires(sizeof...(Values) == Rows * Columns && (std::convertible_to<Values, Scalar> && ...))
    constexpr explicit Matrix(Values&&... entries);

    constexpr Scalar& operator()(std::size_t row, std::size_t column) noexcept { return values_[row][column]; }
    constexpr const Scalar& operator()(std::size_t row, std::size_t column) const noexcept {
        return values_[row][column];
    }
    constexpr bool operator==(const Matrix&) const = default;

   private:
    std::array<std::array<Scalar, Columns>, Rows> values_{};
};

template <std::size_t Dimension>
using SquareMatrix = Matrix<Dimension, Dimension>;

template <std::size_t Dimension>
constexpr SquareMatrix<Dimension> identity_matrix();

template <std::size_t Rows, std::size_t Columns>
constexpr Matrix<Columns, Rows> transposed(const Matrix<Rows, Columns>& matrix);

template <std::size_t Rows, std::size_t Inner, std::size_t Columns>
constexpr Matrix<Rows, Columns> operator*(const Matrix<Rows, Inner>& left, const Matrix<Inner, Columns>& right);

template <std::size_t Rows, std::size_t Columns>
constexpr Vec<Rows> operator*(const Matrix<Rows, Columns>& matrix, const Vec<Columns>& vector);

SquareMatrix<2> inverse(const SquareMatrix<2>& matrix);
SquareMatrix<3> inverse(const SquareMatrix<3>& matrix);

// Implementation ======================================================================================================

template <std::size_t Rows, std::size_t Columns>
template <class... Values>
    requires(sizeof...(Values) == Rows * Columns && (std::convertible_to<Values, Scalar> && ...))
constexpr Matrix<Rows, Columns>::Matrix(Values&&... entries) {
    const std::array<Scalar, Rows * Columns> flat{static_cast<Scalar>(std::forward<Values>(entries))...};
    for (std::size_t row = 0; row < Rows; ++row) {
        for (std::size_t column = 0; column < Columns; ++column) {
            values_[row][column] = flat[row * Columns + column];
        }
    }
}

template <std::size_t Dimension>
constexpr SquareMatrix<Dimension> identity_matrix() {
    SquareMatrix<Dimension> result;
    for (std::size_t i = 0; i < Dimension; ++i) {
        result(i, i) = Scalar{1};
    }
    return result;
}

template <std::size_t Rows, std::size_t Columns>
constexpr Matrix<Columns, Rows> transposed(const Matrix<Rows, Columns>& matrix) {
    Matrix<Columns, Rows> result;
    for (std::size_t row = 0; row < Rows; ++row) {
        for (std::size_t column = 0; column < Columns; ++column) {
            result(column, row) = matrix(row, column);
        }
    }
    return result;
}

template <std::size_t Rows, std::size_t Inner, std::size_t Columns>
constexpr Matrix<Rows, Columns> operator*(const Matrix<Rows, Inner>& left, const Matrix<Inner, Columns>& right) {
    Matrix<Rows, Columns> result;
    for (std::size_t row = 0; row < Rows; ++row) {
        for (std::size_t column = 0; column < Columns; ++column) {
            for (std::size_t inner = 0; inner < Inner; ++inner) {
                result(row, column) += left(row, inner) * right(inner, column);
            }
        }
    }
    return result;
}

template <std::size_t Rows, std::size_t Columns>
constexpr Vec<Rows> operator*(const Matrix<Rows, Columns>& matrix, const Vec<Columns>& vector) {
    Vec<Rows> result{};
    for (std::size_t row = 0; row < Rows; ++row) {
        for (std::size_t column = 0; column < Columns; ++column) {
            result[row] += matrix(row, column) * vector[column];
        }
    }
    return result;
}

inline SquareMatrix<2> inverse(const SquareMatrix<2>& matrix) {
    const Scalar determinant = matrix(0, 0) * matrix(1, 1) - matrix(0, 1) * matrix(1, 0);
    if (std::abs(determinant) <= std::numeric_limits<Scalar>::epsilon()) {
        throw std::invalid_argument("Cannot invert a singular matrix");
    }
    return SquareMatrix<2>{matrix(1, 1) / determinant, -matrix(0, 1) / determinant, -matrix(1, 0) / determinant,
                           matrix(0, 0) / determinant};
}

inline SquareMatrix<3> inverse(const SquareMatrix<3>& matrix) {
    const Scalar determinant = matrix(0, 0) * (matrix(1, 1) * matrix(2, 2) - matrix(1, 2) * matrix(2, 1)) -
                               matrix(0, 1) * (matrix(1, 0) * matrix(2, 2) - matrix(1, 2) * matrix(2, 0)) +
                               matrix(0, 2) * (matrix(1, 0) * matrix(2, 1) - matrix(1, 1) * matrix(2, 0));
    if (std::abs(determinant) <= std::numeric_limits<Scalar>::epsilon()) {
        throw std::invalid_argument("Cannot invert a singular matrix");
    }
    return SquareMatrix<3>{(matrix(1, 1) * matrix(2, 2) - matrix(1, 2) * matrix(2, 1)) / determinant,
                           (matrix(0, 2) * matrix(2, 1) - matrix(0, 1) * matrix(2, 2)) / determinant,
                           (matrix(0, 1) * matrix(1, 2) - matrix(0, 2) * matrix(1, 1)) / determinant,
                           (matrix(1, 2) * matrix(2, 0) - matrix(1, 0) * matrix(2, 2)) / determinant,
                           (matrix(0, 0) * matrix(2, 2) - matrix(0, 2) * matrix(2, 0)) / determinant,
                           (matrix(0, 2) * matrix(1, 0) - matrix(0, 0) * matrix(1, 2)) / determinant,
                           (matrix(1, 0) * matrix(2, 1) - matrix(1, 1) * matrix(2, 0)) / determinant,
                           (matrix(0, 1) * matrix(2, 0) - matrix(0, 0) * matrix(2, 1)) / determinant,
                           (matrix(0, 0) * matrix(1, 1) - matrix(0, 1) * matrix(1, 0)) / determinant};
}

}  // namespace spatia
