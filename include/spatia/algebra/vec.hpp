#pragma once

#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <utility>

#include "spatia/config.hpp"

namespace spatia {

/// Algebraic vector without coordinate-system semantics.
template <std::size_t Dimension>
struct Vec {
    static_assert(Dimension > 0);

    std::array<Scalar, Dimension> values{};

    constexpr Vec() = default;

    template <class... Values>
        requires(sizeof...(Values) == Dimension && (std::convertible_to<Values, Scalar> && ...))
    constexpr explicit Vec(Values&&... coordinates)
        : values{static_cast<Scalar>(std::forward<Values>(coordinates))...} {}

    constexpr Scalar& operator[](std::size_t index) noexcept { return values[index]; }
    constexpr const Scalar& operator[](std::size_t index) const noexcept { return values[index]; }
    constexpr bool operator==(const Vec&) const = default;
};

template <std::size_t Dimension>
constexpr Vec<Dimension> operator+(const Vec<Dimension>& left, const Vec<Dimension>& right);

template <std::size_t Dimension>
constexpr Vec<Dimension> operator-(const Vec<Dimension>& left, const Vec<Dimension>& right);

template <std::size_t Dimension>
constexpr Vec<Dimension> operator-(const Vec<Dimension>& vector);

template <std::size_t Dimension>
constexpr Vec<Dimension> operator*(const Vec<Dimension>& vector, Scalar scale);

template <std::size_t Dimension>
constexpr Vec<Dimension> operator*(Scalar scale, const Vec<Dimension>& vector);

template <std::size_t Dimension>
constexpr Vec<Dimension> operator/(const Vec<Dimension>& vector, Scalar scale);

template <std::size_t Dimension>
constexpr Scalar dot(const Vec<Dimension>& left, const Vec<Dimension>& right);

template <std::size_t Dimension>
Scalar norm(const Vec<Dimension>& vector);

// Implementation ======================================================================================================

template <std::size_t Dimension>
constexpr Vec<Dimension> operator+(const Vec<Dimension>& left, const Vec<Dimension>& right) {
    Vec<Dimension> result;
    for (std::size_t i = 0; i < Dimension; ++i) {
        result[i] = left[i] + right[i];
    }
    return result;
}

template <std::size_t Dimension>
constexpr Vec<Dimension> operator-(const Vec<Dimension>& left, const Vec<Dimension>& right) {
    Vec<Dimension> result;
    for (std::size_t i = 0; i < Dimension; ++i) {
        result[i] = left[i] - right[i];
    }
    return result;
}

template <std::size_t Dimension>
constexpr Vec<Dimension> operator-(const Vec<Dimension>& vector) {
    Vec<Dimension> result;
    for (std::size_t i = 0; i < Dimension; ++i) {
        result[i] = -vector[i];
    }
    return result;
}

template <std::size_t Dimension>
constexpr Vec<Dimension> operator*(const Vec<Dimension>& vector, Scalar scale) {
    Vec<Dimension> result;
    for (std::size_t i = 0; i < Dimension; ++i) {
        result[i] = vector[i] * scale;
    }
    return result;
}

template <std::size_t Dimension>
constexpr Vec<Dimension> operator*(Scalar scale, const Vec<Dimension>& vector) {
    return vector * scale;
}

template <std::size_t Dimension>
constexpr Vec<Dimension> operator/(const Vec<Dimension>& vector, Scalar scale) {
    Vec<Dimension> result;
    for (std::size_t i = 0; i < Dimension; ++i) {
        result[i] = vector[i] / scale;
    }
    return result;
}

template <std::size_t Dimension>
constexpr Scalar dot(const Vec<Dimension>& left, const Vec<Dimension>& right) {
    Scalar result{};
    for (std::size_t i = 0; i < Dimension; ++i) {
        result += left[i] * right[i];
    }
    return result;
}

template <std::size_t Dimension>
Scalar norm(const Vec<Dimension>& vector) {
    return std::sqrt(dot(vector, vector));
}

}  // namespace spatia
