#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <utility>

#include "geometry2/math/vector.hpp"

namespace geometry2::math {

template <std::size_t Dimension, class Scalar = double, class Tag = void>
struct Point {
    static_assert(std::floating_point<Scalar>);

    std::array<Scalar, Dimension> values{};

    constexpr Point();

    template <class... Values>
        requires(sizeof...(Values) == Dimension &&
                 (std::convertible_to<Values, Scalar> && ...))
    constexpr explicit Point(Values&&... coordinates);

    constexpr Scalar& operator[](std::size_t index) noexcept;
    constexpr const Scalar& operator[](std::size_t index) const noexcept;

    constexpr Scalar& x() noexcept
        requires(Dimension >= 1);
    constexpr const Scalar& x() const noexcept
        requires(Dimension >= 1);
    constexpr Scalar& y() noexcept
        requires(Dimension >= 2);
    constexpr const Scalar& y() const noexcept
        requires(Dimension >= 2);
    constexpr Scalar& z() noexcept
        requires(Dimension >= 3);
    constexpr const Scalar& z() const noexcept
        requires(Dimension >= 3);

    constexpr bool operator==(const Point&) const;
};

template <class Scalar = double, class Tag = void>
using Point2 = Point<2, Scalar, Tag>;

template <class Scalar = double, class Tag = void>
using Point3 = Point<3, Scalar, Tag>;

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator-(const Point<Dimension, Scalar, Tag>& left,
                         const Point<Dimension, Scalar, Tag>& right)
    -> Vector<Dimension, Scalar, Tag>;

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator+(const Point<Dimension, Scalar, Tag>& point,
                         const Vector<Dimension, Scalar, Tag>& offset)
    -> Point<Dimension, Scalar, Tag>;

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator+(const Vector<Dimension, Scalar, Tag>& offset,
                         const Point<Dimension, Scalar, Tag>& point)
    -> Point<Dimension, Scalar, Tag>;

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator-(const Point<Dimension, Scalar, Tag>& point,
                         const Vector<Dimension, Scalar, Tag>& offset)
    -> Point<Dimension, Scalar, Tag>;

// Implementation

template <std::size_t Dimension, class Scalar, class Tag>
constexpr Point<Dimension, Scalar, Tag>::Point() = default;

template <std::size_t Dimension, class Scalar, class Tag>
template <class... Values>
    requires(sizeof...(Values) == Dimension &&
             (std::convertible_to<Values, Scalar> && ...))
constexpr Point<Dimension, Scalar, Tag>::Point(Values&&... coordinates)
    : values{static_cast<Scalar>(std::forward<Values>(coordinates))...} {}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr Scalar& Point<Dimension, Scalar, Tag>::operator[](
    std::size_t index) noexcept {
    return values[index];
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr const Scalar& Point<Dimension, Scalar, Tag>::operator[](
    std::size_t index) const noexcept {
    return values[index];
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr Scalar& Point<Dimension, Scalar, Tag>::x() noexcept
    requires(Dimension >= 1)
{
    return values[0];
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr const Scalar& Point<Dimension, Scalar, Tag>::x() const noexcept
    requires(Dimension >= 1)
{
    return values[0];
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr Scalar& Point<Dimension, Scalar, Tag>::y() noexcept
    requires(Dimension >= 2)
{
    return values[1];
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr const Scalar& Point<Dimension, Scalar, Tag>::y() const noexcept
    requires(Dimension >= 2)
{
    return values[1];
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr Scalar& Point<Dimension, Scalar, Tag>::z() noexcept
    requires(Dimension >= 3)
{
    return values[2];
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr const Scalar& Point<Dimension, Scalar, Tag>::z() const noexcept
    requires(Dimension >= 3)
{
    return values[2];
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr bool Point<Dimension, Scalar, Tag>::operator==(const Point&) const =
    default;

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator-(const Point<Dimension, Scalar, Tag>& left,
                         const Point<Dimension, Scalar, Tag>& right)
    -> Vector<Dimension, Scalar, Tag> {
    Vector<Dimension, Scalar, Tag> result;
    for (std::size_t i = 0; i < Dimension; ++i) {
        result[i] = left[i] - right[i];
    }
    return result;
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator+(const Point<Dimension, Scalar, Tag>& point,
                         const Vector<Dimension, Scalar, Tag>& offset)
    -> Point<Dimension, Scalar, Tag> {
    Point<Dimension, Scalar, Tag> result;
    for (std::size_t i = 0; i < Dimension; ++i) {
        result[i] = point[i] + offset[i];
    }
    return result;
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator+(const Vector<Dimension, Scalar, Tag>& offset,
                         const Point<Dimension, Scalar, Tag>& point)
    -> Point<Dimension, Scalar, Tag> {
    return point + offset;
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator-(const Point<Dimension, Scalar, Tag>& point,
                         const Vector<Dimension, Scalar, Tag>& offset)
    -> Point<Dimension, Scalar, Tag> {
    return point + (-offset);
}

}  // namespace geometry2::math
