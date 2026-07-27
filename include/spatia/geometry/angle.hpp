#pragma once

#include <cmath>
#include <numbers>

#include "spatia/config.hpp"

namespace spatia {

/// Semantic angle with explicit unit factories.
class Angle {
   public:
    constexpr Angle() = default;

    static constexpr Angle from_radians(Scalar radians) noexcept { return Angle{radians}; }
    static constexpr Angle from_degrees(Scalar degrees) noexcept {
        return Angle{degrees * std::numbers::pi_v<Scalar> / Scalar{180}};
    }

    constexpr Scalar to_radians() const noexcept { return radians_; }
    constexpr Scalar to_degrees() const noexcept { return radians_ * Scalar{180} / std::numbers::pi_v<Scalar>; }

    Angle normalized_unsigned() const noexcept;
    Angle normalized_signed() const noexcept;

    constexpr Angle& operator+=(Angle other) noexcept {
        radians_ += other.radians_;
        return *this;
    }
    constexpr Angle& operator-=(Angle other) noexcept {
        radians_ -= other.radians_;
        return *this;
    }
    constexpr bool operator==(const Angle&) const = default;

   private:
    explicit constexpr Angle(Scalar radians) noexcept : radians_(radians) {}

    Scalar radians_{};
};

constexpr Angle operator+(Angle left, Angle right) noexcept;
constexpr Angle operator-(Angle left, Angle right) noexcept;
constexpr Angle operator-(Angle angle) noexcept;
constexpr Angle operator*(Angle angle, Scalar scale) noexcept;
constexpr Angle operator*(Scalar scale, Angle angle) noexcept;
constexpr Angle operator/(Angle angle, Scalar scale) noexcept;

// Implementation ======================================================================================================

/// Returns the angle normalized to [0, 2*pi).
inline Angle Angle::normalized_unsigned() const noexcept {
    constexpr Scalar full_turn = Scalar{2} * std::numbers::pi_v<Scalar>;
    Scalar result = std::fmod(radians_, full_turn);
    if (result < Scalar{}) {
        result += full_turn;
    }
    return Angle{result};
}

/// Returns the angle normalized to [-pi, pi).
inline Angle Angle::normalized_signed() const noexcept {
    const Scalar shifted = (Angle{radians_ + std::numbers::pi_v<Scalar>}.normalized_unsigned()).to_radians();
    return Angle{shifted - std::numbers::pi_v<Scalar>};
}

constexpr Angle operator+(Angle left, Angle right) noexcept {
    left += right;
    return left;
}

constexpr Angle operator-(Angle left, Angle right) noexcept {
    left -= right;
    return left;
}

constexpr Angle operator-(Angle angle) noexcept { return Angle::from_radians(-angle.to_radians()); }

constexpr Angle operator*(Angle angle, Scalar scale) noexcept {
    return Angle::from_radians(angle.to_radians() * scale);
}

constexpr Angle operator*(Scalar scale, Angle angle) noexcept { return angle * scale; }

constexpr Angle operator/(Angle angle, Scalar scale) noexcept {
    return Angle::from_radians(angle.to_radians() / scale);
}

}  // namespace spatia
