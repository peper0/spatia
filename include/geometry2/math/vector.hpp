#pragma once

#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <utility>

namespace geometry2::math {

template <std::size_t Dimension, class Scalar = double, class Tag = void>
struct Vector {
    static_assert(std::floating_point<Scalar>);

    std::array<Scalar, Dimension> values{};

    constexpr Vector();

    template <class... Values>
        requires(sizeof...(Values) == Dimension &&
                 (std::convertible_to<Values, Scalar> && ...))
    constexpr explicit Vector(Values&&... coordinates);

    constexpr Scalar& operator[](std::size_t index) noexcept;
    constexpr const Scalar& operator[](std::size_t index) const noexcept;

    constexpr Scalar& dx() noexcept
        requires(Dimension >= 1);
    constexpr const Scalar& dx() const noexcept
        requires(Dimension >= 1);
    constexpr Scalar& dy() noexcept
        requires(Dimension >= 2);
    constexpr const Scalar& dy() const noexcept
        requires(Dimension >= 2);
    constexpr Scalar& dz() noexcept
        requires(Dimension >= 3);
    constexpr const Scalar& dz() const noexcept
        requires(Dimension >= 3);

    constexpr bool operator==(const Vector&) const;
};

template <class Scalar = double, class Tag = void>
using Vector2 = Vector<2, Scalar, Tag>;

template <class Scalar = double, class Tag = void>
using Vector3 = Vector<3, Scalar, Tag>;

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator+(const Vector<Dimension, Scalar, Tag>& left,
                         const Vector<Dimension, Scalar, Tag>& right)
    -> Vector<Dimension, Scalar, Tag>;

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator-(const Vector<Dimension, Scalar, Tag>& left,
                         const Vector<Dimension, Scalar, Tag>& right)
    -> Vector<Dimension, Scalar, Tag>;

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator-(const Vector<Dimension, Scalar, Tag>& vector)
    -> Vector<Dimension, Scalar, Tag>;

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator*(const Vector<Dimension, Scalar, Tag>& vector,
                         Scalar scale) -> Vector<Dimension, Scalar, Tag>;

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator*(Scalar scale,
                         const Vector<Dimension, Scalar, Tag>& vector)
    -> Vector<Dimension, Scalar, Tag>;

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator/(const Vector<Dimension, Scalar, Tag>& vector,
                         Scalar scale) -> Vector<Dimension, Scalar, Tag>;

template <std::size_t Dimension, class Scalar, class Tag>
constexpr Scalar dot(const Vector<Dimension, Scalar, Tag>& left,
                     const Vector<Dimension, Scalar, Tag>& right);

template <std::size_t Dimension, class Scalar, class Tag>
Scalar norm(const Vector<Dimension, Scalar, Tag>& vector);

// Implementation

template <std::size_t Dimension, class Scalar, class Tag>
constexpr Vector<Dimension, Scalar, Tag>::Vector() = default;

template <std::size_t Dimension, class Scalar, class Tag>
template <class... Values>
    requires(sizeof...(Values) == Dimension &&
             (std::convertible_to<Values, Scalar> && ...))
constexpr Vector<Dimension, Scalar, Tag>::Vector(Values&&... coordinates)
    : values{static_cast<Scalar>(std::forward<Values>(coordinates))...} {}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr Scalar& Vector<Dimension, Scalar, Tag>::operator[](
    std::size_t index) noexcept {
    return values[index];
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr const Scalar& Vector<Dimension, Scalar, Tag>::operator[](
    std::size_t index) const noexcept {
    return values[index];
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr Scalar& Vector<Dimension, Scalar, Tag>::dx() noexcept
    requires(Dimension >= 1)
{
    return values[0];
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr const Scalar& Vector<Dimension, Scalar, Tag>::dx() const noexcept
    requires(Dimension >= 1)
{
    return values[0];
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr Scalar& Vector<Dimension, Scalar, Tag>::dy() noexcept
    requires(Dimension >= 2)
{
    return values[1];
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr const Scalar& Vector<Dimension, Scalar, Tag>::dy() const noexcept
    requires(Dimension >= 2)
{
    return values[1];
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr Scalar& Vector<Dimension, Scalar, Tag>::dz() noexcept
    requires(Dimension >= 3)
{
    return values[2];
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr const Scalar& Vector<Dimension, Scalar, Tag>::dz() const noexcept
    requires(Dimension >= 3)
{
    return values[2];
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr bool Vector<Dimension, Scalar, Tag>::operator==(const Vector&) const =
    default;

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator+(const Vector<Dimension, Scalar, Tag>& left,
                         const Vector<Dimension, Scalar, Tag>& right)
    -> Vector<Dimension, Scalar, Tag> {
    Vector<Dimension, Scalar, Tag> result;
    for (std::size_t i = 0; i < Dimension; ++i) {
        result[i] = left[i] + right[i];
    }
    return result;
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator-(const Vector<Dimension, Scalar, Tag>& left,
                         const Vector<Dimension, Scalar, Tag>& right)
    -> Vector<Dimension, Scalar, Tag> {
    Vector<Dimension, Scalar, Tag> result;
    for (std::size_t i = 0; i < Dimension; ++i) {
        result[i] = left[i] - right[i];
    }
    return result;
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator-(const Vector<Dimension, Scalar, Tag>& vector)
    -> Vector<Dimension, Scalar, Tag> {
    Vector<Dimension, Scalar, Tag> result;
    for (std::size_t i = 0; i < Dimension; ++i) {
        result[i] = -vector[i];
    }
    return result;
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator*(const Vector<Dimension, Scalar, Tag>& vector,
                         Scalar scale) -> Vector<Dimension, Scalar, Tag> {
    Vector<Dimension, Scalar, Tag> result;
    for (std::size_t i = 0; i < Dimension; ++i) {
        result[i] = vector[i] * scale;
    }
    return result;
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator*(Scalar scale,
                         const Vector<Dimension, Scalar, Tag>& vector)
    -> Vector<Dimension, Scalar, Tag> {
    return vector * scale;
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr auto operator/(const Vector<Dimension, Scalar, Tag>& vector,
                         Scalar scale) -> Vector<Dimension, Scalar, Tag> {
    Vector<Dimension, Scalar, Tag> result;
    for (std::size_t i = 0; i < Dimension; ++i) {
        result[i] = vector[i] / scale;
    }
    return result;
}

template <std::size_t Dimension, class Scalar, class Tag>
constexpr Scalar dot(const Vector<Dimension, Scalar, Tag>& left,
                     const Vector<Dimension, Scalar, Tag>& right) {
    Scalar result{};
    for (std::size_t i = 0; i < Dimension; ++i) {
        result += left[i] * right[i];
    }
    return result;
}

template <std::size_t Dimension, class Scalar, class Tag>
Scalar norm(const Vector<Dimension, Scalar, Tag>& vector) {
    return std::sqrt(dot(vector, vector));
}

}  // namespace geometry2::math
