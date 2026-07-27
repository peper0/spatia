#pragma once

#include <array>
#include <cstddef>

#include "spatia/config.hpp"
#include "spatia/geometry/angle.hpp"

namespace spatia {

/// Two consecutive angles about the Z and Y axes of one system, used as a
/// two-angle parameterization of a direction.
template <class System>
    requires(dimension_v<System> == 3)
class EulerZY {
   public:
    using SystemType = System;

    constexpr EulerZY() = default;
    constexpr EulerZY(Angle z, Angle y) noexcept : values_{z, y} {}

    constexpr Angle& operator[](std::size_t index) noexcept { return values_[index]; }
    constexpr const Angle& operator[](std::size_t index) const noexcept { return values_[index]; }
    constexpr Angle& z() noexcept { return values_[0]; }
    constexpr const Angle& z() const noexcept { return values_[0]; }
    constexpr Angle& y() noexcept { return values_[1]; }
    constexpr const Angle& y() const noexcept { return values_[1]; }
    constexpr bool operator==(const EulerZY&) const = default;

   private:
    std::array<Angle, 2> values_{};
};

/// Full rotation between two systems through the intrinsic Z-Y'-X'' sequence
/// of Tait-Bryan angles.
///
/// The dimensions are a constraint rather than a `static_assert`, so that
/// naming `EulerZYX` for a two-dimensional system merely removes an overload
/// from consideration instead of failing the whole translation unit. Overload
/// sets that mix this type with plane rotations depend on that.
template <class From, class To>
    requires(dimension_v<From> == 3 && dimension_v<To> == 3)
class EulerZYX {
   public:
    using FromSystem = From;
    using ToSystem = To;

    constexpr EulerZYX() = default;
    constexpr EulerZYX(Angle z, Angle y, Angle x) noexcept : values_{z, y, x} {}

    constexpr Angle& operator[](std::size_t index) noexcept { return values_[index]; }
    constexpr const Angle& operator[](std::size_t index) const noexcept { return values_[index]; }
    constexpr Angle& z() noexcept { return values_[0]; }
    constexpr const Angle& z() const noexcept { return values_[0]; }
    constexpr Angle& y() noexcept { return values_[1]; }
    constexpr const Angle& y() const noexcept { return values_[1]; }
    constexpr Angle& x() noexcept { return values_[2]; }
    constexpr const Angle& x() const noexcept { return values_[2]; }
    constexpr bool operator==(const EulerZYX&) const = default;

   private:
    std::array<Angle, 3> values_{};
};

}  // namespace spatia
