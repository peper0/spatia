#pragma once

#include <gmock/gmock-matchers.h>

#include <array>
#include <string>

#include "spatia/algebra/matrix.hpp"
#include "spatia/algebra/vec.hpp"

namespace spatia {

/// Matches every matrix entry within the given absolute tolerance.
template <std::size_t Rows, std::size_t Columns>
::testing::Matcher<const Matrix<Rows, Columns>&> matrix_near(const Matrix<Rows, Columns>& expected, Scalar tolerance);

/// Matches every coordinate within the given absolute tolerance.
template <std::size_t Dimension>
::testing::Matcher<const Vec<Dimension>&> coordinates_near(const Vec<Dimension>& expected, Scalar tolerance);

/// Matches points or vectors of the same type, including their coordinate system.
template <class Value>
    requires requires(const Value& value) { value.to_vec(); }
::testing::Matcher<const Value&> coordinates_near(const Value& expected, Scalar tolerance);

// Implementation ======================================================================================================

template <std::size_t Rows, std::size_t Columns>
::testing::Matcher<const Matrix<Rows, Columns>&> matrix_near(const Matrix<Rows, Columns>& expected, Scalar tolerance) {
    using Value = Matrix<Rows, Columns>;
    std::array<::testing::Matcher<const Value&>, Rows * Columns> entries;
    for (std::size_t row = 0; row < Rows; ++row) {
        for (std::size_t column = 0; column < Columns; ++column) {
            entries[row * Columns + column] = ::testing::ResultOf(
                "entry (" + std::to_string(row) + ", " + std::to_string(column) + ")",
                [row, column](const Value& value) { return value(row, column); },
                ::testing::DoubleNear(expected(row, column), tolerance));
        }
    }
    return ::testing::AllOfArray(entries);
}

template <std::size_t Dimension>
::testing::Matcher<const Vec<Dimension>&> coordinates_near(const Vec<Dimension>& expected, Scalar tolerance) {
    return ::testing::Field("coordinates", &Vec<Dimension>::values,
                            ::testing::Pointwise(::testing::DoubleNear(tolerance), expected.values));
}

template <class Value>
    requires requires(const Value& value) { value.to_vec(); }
::testing::Matcher<const Value&> coordinates_near(const Value& expected, Scalar tolerance) {
    return ::testing::ResultOf("coordinates", [](const Value& value) { return value.to_vec(); },
                               coordinates_near(expected.to_vec(), tolerance));
}

}  // namespace spatia
