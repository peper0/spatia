#pragma once

#include <concepts>

namespace geometry2::math {

// Strong angle type. The stored value and all arithmetic are in radians;
// construction from a scalar is explicit so unitless numbers cannot be passed
// accidentally to APIs expecting an angle.
template <std::floating_point Scalar = double>
class Radians {
   public:
    using scalar_type = Scalar;

    constexpr Radians();
    constexpr explicit Radians(Scalar value);

    constexpr Scalar value() const noexcept;
    constexpr bool operator==(const Radians&) const;

   private:
    Scalar value_{};
};

template <std::floating_point Scalar>
constexpr Radians<Scalar> operator+(Radians<Scalar> left,
                                    Radians<Scalar> right);

template <std::floating_point Scalar>
constexpr Radians<Scalar> operator-(Radians<Scalar> left,
                                    Radians<Scalar> right);

template <std::floating_point Scalar>
constexpr Radians<Scalar> operator-(Radians<Scalar> angle);

template <std::floating_point Scalar, class Scale>
    requires std::convertible_to<Scale, Scalar>
constexpr Radians<Scalar> operator*(Radians<Scalar> angle, Scale scale);

template <class Scale, std::floating_point Scalar>
    requires std::convertible_to<Scale, Scalar>
constexpr Radians<Scalar> operator*(Scale scale, Radians<Scalar> angle);

template <std::floating_point Scalar, class Scale>
    requires std::convertible_to<Scale, Scalar>
constexpr Radians<Scalar> operator/(Radians<Scalar> angle, Scale scale);

// Implementation

template <std::floating_point Scalar>
constexpr Radians<Scalar>::Radians() = default;

template <std::floating_point Scalar>
constexpr Radians<Scalar>::Radians(Scalar value) : value_(value) {}

template <std::floating_point Scalar>
constexpr Scalar Radians<Scalar>::value() const noexcept {
    return value_;
}

template <std::floating_point Scalar>
constexpr bool Radians<Scalar>::operator==(const Radians&) const = default;

template <std::floating_point Scalar>
constexpr Radians<Scalar> operator+(Radians<Scalar> left,
                                    Radians<Scalar> right) {
    return Radians<Scalar>{left.value() + right.value()};
}

template <std::floating_point Scalar>
constexpr Radians<Scalar> operator-(Radians<Scalar> left,
                                    Radians<Scalar> right) {
    return Radians<Scalar>{left.value() - right.value()};
}

template <std::floating_point Scalar>
constexpr Radians<Scalar> operator-(Radians<Scalar> angle) {
    return Radians<Scalar>{-angle.value()};
}

template <std::floating_point Scalar, class Scale>
    requires std::convertible_to<Scale, Scalar>
constexpr Radians<Scalar> operator*(Radians<Scalar> angle, Scale scale) {
    return Radians<Scalar>{angle.value() * static_cast<Scalar>(scale)};
}

template <class Scale, std::floating_point Scalar>
    requires std::convertible_to<Scale, Scalar>
constexpr Radians<Scalar> operator*(Scale scale, Radians<Scalar> angle) {
    return angle * scale;
}

template <std::floating_point Scalar, class Scale>
    requires std::convertible_to<Scale, Scalar>
constexpr Radians<Scalar> operator/(Radians<Scalar> angle, Scale scale) {
    return Radians<Scalar>{angle.value() / static_cast<Scalar>(scale)};
}

}  // namespace geometry2::math
