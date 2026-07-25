#pragma once

#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

namespace geometry2::math {

template <std::size_t Rows, std::size_t Columns, class Scalar = double>
class Matrix {
    static_assert(Rows > 0 && Columns > 0);
    static_assert(std::floating_point<Scalar>);

   public:
    using scalar_type = Scalar;
    static constexpr std::size_t row_count = Rows;
    static constexpr std::size_t column_count = Columns;

    constexpr Matrix();

    template <class... Values>
        requires(sizeof...(Values) == Rows * Columns &&
                 (std::convertible_to<Values, Scalar> && ...))
    constexpr explicit Matrix(Values&&... entries);

    constexpr Scalar& operator()(std::size_t row, std::size_t column) noexcept;
    constexpr const Scalar& operator()(std::size_t row,
                                       std::size_t column) const noexcept;
    constexpr bool operator==(const Matrix&) const;

   private:
    std::array<std::array<Scalar, Columns>, Rows> values_{};
};

template <class T>
concept MatrixLike = requires(T matrix, const T constant_matrix,
                              std::size_t row, std::size_t column) {
    typename T::scalar_type;
    { T::row_count } -> std::convertible_to<std::size_t>;
    { T::column_count } -> std::convertible_to<std::size_t>;
    { matrix(row, column) } -> std::same_as<typename T::scalar_type&>;
    {
        constant_matrix(row, column)
    } -> std::convertible_to<typename T::scalar_type>;
};

template <std::size_t Dimension, class Scalar = double>
using SquareMatrix = Matrix<Dimension, Dimension, Scalar>;

template <std::size_t Dimension, class Scalar>
constexpr SquareMatrix<Dimension, Scalar> identity_matrix();

template <MatrixLike MatrixType>
constexpr auto transposed(const MatrixType& matrix);

template <std::size_t Rows, std::size_t Inner, std::size_t Columns,
          class Scalar>
constexpr Matrix<Rows, Columns, Scalar> operator*(
    const Matrix<Rows, Inner, Scalar>& left,
    const Matrix<Inner, Columns, Scalar>& right);

template <class Output, class Input, MatrixLike MatrixType>
constexpr Output apply_matrix(const MatrixType& matrix, const Input& input);

template <class Scalar>
SquareMatrix<2, Scalar> inverse(const SquareMatrix<2, Scalar>& matrix);

template <class Scalar>
SquareMatrix<3, Scalar> inverse(const SquareMatrix<3, Scalar>& matrix);

// Implementation

template <std::size_t Rows, std::size_t Columns, class Scalar>
constexpr Matrix<Rows, Columns, Scalar>::Matrix() = default;

template <std::size_t Rows, std::size_t Columns, class Scalar>
template <class... Values>
    requires(sizeof...(Values) == Rows * Columns &&
             (std::convertible_to<Values, Scalar> && ...))
constexpr Matrix<Rows, Columns, Scalar>::Matrix(Values&&... entries) {
    const std::array<Scalar, Rows * Columns> flat{
        static_cast<Scalar>(std::forward<Values>(entries))...};
    for (std::size_t row = 0; row < Rows; ++row) {
        for (std::size_t column = 0; column < Columns; ++column) {
            values_[row][column] = flat[row * Columns + column];
        }
    }
}

template <std::size_t Rows, std::size_t Columns, class Scalar>
constexpr Scalar& Matrix<Rows, Columns, Scalar>::operator()(
    std::size_t row, std::size_t column) noexcept {
    return values_[row][column];
}

template <std::size_t Rows, std::size_t Columns, class Scalar>
constexpr const Scalar& Matrix<Rows, Columns, Scalar>::operator()(
    std::size_t row, std::size_t column) const noexcept {
    return values_[row][column];
}

template <std::size_t Rows, std::size_t Columns, class Scalar>
constexpr bool Matrix<Rows, Columns, Scalar>::operator==(const Matrix&) const =
    default;

template <std::size_t Dimension, class Scalar>
constexpr SquareMatrix<Dimension, Scalar> identity_matrix() {
    SquareMatrix<Dimension, Scalar> result;
    for (std::size_t i = 0; i < Dimension; ++i) {
        result(i, i) = Scalar{1};
    }
    return result;
}

template <MatrixLike MatrixType>
constexpr auto transposed(const MatrixType& matrix) {
    using Scalar = typename MatrixType::scalar_type;
    Matrix<MatrixType::column_count, MatrixType::row_count, Scalar> result;
    for (std::size_t row = 0; row < MatrixType::row_count; ++row) {
        for (std::size_t column = 0; column < MatrixType::column_count;
             ++column) {
            result(column, row) = matrix(row, column);
        }
    }
    return result;
}

template <std::size_t Rows, std::size_t Inner, std::size_t Columns,
          class Scalar>
constexpr Matrix<Rows, Columns, Scalar> operator*(
    const Matrix<Rows, Inner, Scalar>& left,
    const Matrix<Inner, Columns, Scalar>& right) {
    Matrix<Rows, Columns, Scalar> result;
    for (std::size_t row = 0; row < Rows; ++row) {
        for (std::size_t column = 0; column < Columns; ++column) {
            for (std::size_t inner = 0; inner < Inner; ++inner) {
                result(row, column) += left(row, inner) * right(inner, column);
            }
        }
    }
    return result;
}

template <class Output, class Input, MatrixLike MatrixType>
constexpr Output apply_matrix(const MatrixType& matrix, const Input& input) {
    Output result{};
    for (std::size_t row = 0; row < MatrixType::row_count; ++row) {
        for (std::size_t column = 0; column < MatrixType::column_count;
             ++column) {
            result[row] += matrix(row, column) * input[column];
        }
    }
    return result;
}

template <class Scalar>
SquareMatrix<2, Scalar> inverse(const SquareMatrix<2, Scalar>& matrix) {
    const Scalar determinant =
        matrix(0, 0) * matrix(1, 1) - matrix(0, 1) * matrix(1, 0);
    if (std::abs(determinant) <= std::numeric_limits<Scalar>::epsilon()) {
        throw std::invalid_argument("Cannot invert a singular matrix");
    }

    return SquareMatrix<2, Scalar>{
        matrix(1, 1) / determinant, -matrix(0, 1) / determinant,
        -matrix(1, 0) / determinant, matrix(0, 0) / determinant};
}

template <class Scalar>
SquareMatrix<3, Scalar> inverse(const SquareMatrix<3, Scalar>& matrix) {
    const Scalar determinant =
        matrix(0, 0) *
            (matrix(1, 1) * matrix(2, 2) - matrix(1, 2) * matrix(2, 1)) -
        matrix(0, 1) *
            (matrix(1, 0) * matrix(2, 2) - matrix(1, 2) * matrix(2, 0)) +
        matrix(0, 2) *
            (matrix(1, 0) * matrix(2, 1) - matrix(1, 1) * matrix(2, 0));
    if (std::abs(determinant) <= std::numeric_limits<Scalar>::epsilon()) {
        throw std::invalid_argument("Cannot invert a singular matrix");
    }

    return SquareMatrix<3, Scalar>{
        (matrix(1, 1) * matrix(2, 2) - matrix(1, 2) * matrix(2, 1)) /
            determinant,
        (matrix(0, 2) * matrix(2, 1) - matrix(0, 1) * matrix(2, 2)) /
            determinant,
        (matrix(0, 1) * matrix(1, 2) - matrix(0, 2) * matrix(1, 1)) /
            determinant,
        (matrix(1, 2) * matrix(2, 0) - matrix(1, 0) * matrix(2, 2)) /
            determinant,
        (matrix(0, 0) * matrix(2, 2) - matrix(0, 2) * matrix(2, 0)) /
            determinant,
        (matrix(0, 2) * matrix(1, 0) - matrix(0, 0) * matrix(1, 2)) /
            determinant,
        (matrix(1, 0) * matrix(2, 1) - matrix(1, 1) * matrix(2, 0)) /
            determinant,
        (matrix(0, 1) * matrix(2, 0) - matrix(0, 0) * matrix(2, 1)) /
            determinant,
        (matrix(0, 0) * matrix(1, 1) - matrix(0, 1) * matrix(1, 0)) /
            determinant};
}

}  // namespace geometry2::math
