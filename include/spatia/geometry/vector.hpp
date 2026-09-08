#pragma once

#include <concepts>
#include <cstddef>
#include <utility>

#include "spatia/algebra/vec.hpp"
#include "spatia/config.hpp"

namespace spatia {

/// Displacement expressed in a concrete coordinate system. A type of difference between two points.
template <class System>
class Vector {
    static_assert(dimension_v<System> > 0);

   public:
    static constexpr std::size_t dimension = dimension_v<System>;

    constexpr Vector() = default;

    template <class... Values>
        requires(sizeof...(Values) == dimension_v<System> && (std::convertible_to<Values, Scalar> && ...))
    constexpr explicit Vector(Values&&... coordinates) : coordinates_(std::forward<Values>(coordinates)...) {}

    constexpr explicit Vector(const Vec<dimension>& coordinates) : coordinates_(coordinates) {}
    constexpr Vec<dimension> to_vec() const { return coordinates_; }

    constexpr Scalar& operator[](std::size_t index) noexcept { return coordinates_[index]; }
    constexpr const Scalar& operator[](std::size_t index) const noexcept { return coordinates_[index]; }
    constexpr Scalar& dx() noexcept
        requires(dimension >= 1)
    {
        return coordinates_[0];
    }
    constexpr const Scalar& dx() const noexcept
        requires(dimension >= 1)
    {
        return coordinates_[0];
    }
    constexpr Scalar& dy() noexcept
        requires(dimension >= 2)
    {
        return coordinates_[1];
    }
    constexpr const Scalar& dy() const noexcept
        requires(dimension >= 2)
    {
        return coordinates_[1];
    }
    constexpr Scalar& dz() noexcept
        requires(dimension >= 3)
    {
        return coordinates_[2];
    }
    constexpr const Scalar& dz() const noexcept
        requires(dimension >= 3)
    {
        return coordinates_[2];
    }
    constexpr bool operator==(const Vector&) const = default;

   private:
    Vec<dimension> coordinates_{};
};

template <class System>
constexpr Vector<System> operator+(const Vector<System>& left, const Vector<System>& right);

template <class System>
constexpr Vector<System> operator-(const Vector<System>& left, const Vector<System>& right);

template <class System>
constexpr Vector<System> operator-(const Vector<System>& vector);

template <class System>
constexpr Vector<System> operator*(const Vector<System>& vector, Scalar scale);

template <class System>
constexpr Vector<System> operator*(Scalar scale, const Vector<System>& vector);

template <class System>
constexpr Vector<System> operator/(const Vector<System>& vector, Scalar scale);

template <class System>
constexpr Scalar dot(const Vector<System>& left, const Vector<System>& right);

template <class System>
Scalar norm(const Vector<System>& vector);

// Implementation ======================================================================================================

template <class System>
constexpr Vector<System> operator+(const Vector<System>& left, const Vector<System>& right) {
    return Vector<System>{left.to_vec() + right.to_vec()};
}

template <class System>
constexpr Vector<System> operator-(const Vector<System>& left, const Vector<System>& right) {
    return Vector<System>{left.to_vec() - right.to_vec()};
}

template <class System>
constexpr Vector<System> operator-(const Vector<System>& vector) {
    return Vector<System>{-vector.to_vec()};
}

template <class System>
constexpr Vector<System> operator*(const Vector<System>& vector, Scalar scale) {
    return Vector<System>{vector.to_vec() * scale};
}

template <class System>
constexpr Vector<System> operator*(Scalar scale, const Vector<System>& vector) {
    return vector * scale;
}

template <class System>
constexpr Vector<System> operator/(const Vector<System>& vector, Scalar scale) {
    return Vector<System>{vector.to_vec() / scale};
}

template <class System>
constexpr Scalar dot(const Vector<System>& left, const Vector<System>& right) {
    return dot(left.to_vec(), right.to_vec());
}

template <class System>
Scalar norm(const Vector<System>& vector) {
    return norm(vector.to_vec());
}

}  // namespace spatia
