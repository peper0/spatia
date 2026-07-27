#pragma once

#include <concepts>
#include <cstddef>
#include <utility>

#include "spatia/algebra/vec.hpp"
#include "spatia/config.hpp"
#include "spatia/geometry/vector.hpp"

namespace spatia {

/// Position in a concrete coordinate system.
template <class System>
class Point {
    static_assert(dimension_v<System> > 0);

   public:
    static constexpr std::size_t dimension = dimension_v<System>;

    constexpr Point() = default;

    template <class... Values>
        requires(sizeof...(Values) == dimension_v<System> && (std::convertible_to<Values, Scalar> && ...))
    constexpr explicit Point(Values&&... coordinates) : coordinates_(std::forward<Values>(coordinates)...) {}

    static constexpr Point from_vec(const Vec<dimension>& coordinates) {  // AI to jako explicit konstruktor
        Point result;
        result.coordinates_ = coordinates;
        return result;
    }
    constexpr Vec<dimension> to_vec() const { return coordinates_; }

    constexpr Scalar& operator[](std::size_t index) noexcept { return coordinates_[index]; }
    constexpr const Scalar& operator[](std::size_t index) const noexcept { return coordinates_[index]; }
    constexpr Scalar& x() noexcept
        requires(dimension >= 1)
    {
        return coordinates_[0];
    }

    constexpr const Scalar& x() const noexcept
        requires(dimension >= 1)
    {
        return coordinates_[0];
    }
    constexpr Scalar& y() noexcept
        requires(dimension >= 2)
    {
        return coordinates_[1];
    }
    constexpr const Scalar& y() const noexcept
        requires(dimension >= 2)
    {
        return coordinates_[1];
    }
    constexpr Scalar& z() noexcept
        requires(dimension >= 3)
    {
        return coordinates_[2];
    }
    constexpr const Scalar& z() const noexcept
        requires(dimension >= 3)
    {
        return coordinates_[2];
    }
    constexpr bool operator==(const Point&) const = default;

   private:
    Vec<dimension> coordinates_{};
};

template <class System>
constexpr Vector<System> operator-(const Point<System>& left, const Point<System>& right);

template <class System>
constexpr Point<System> operator+(const Point<System>& point, const Vector<System>& offset);

template <class System>
constexpr Point<System> operator+(const Vector<System>& offset, const Point<System>& point);

template <class System>
constexpr Point<System> operator-(const Point<System>& point, const Vector<System>& offset);

// Implementation ======================================================================================================

template <class System>
constexpr Vector<System> operator-(const Point<System>& left, const Point<System>& right) {
    return Vector<System>::from_vec(left.to_vec() - right.to_vec());
}

template <class System>
constexpr Point<System> operator+(const Point<System>& point, const Vector<System>& offset) {
    return Point<System>::from_vec(point.to_vec() + offset.to_vec());
}

template <class System>
constexpr Point<System> operator+(const Vector<System>& offset, const Point<System>& point) {
    return point + offset;
}

template <class System>
constexpr Point<System> operator-(const Point<System>& point, const Vector<System>& offset) {
    return point + (-offset);
}

}  // namespace spatia
