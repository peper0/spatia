#pragma once

#include <array>
#include <cstddef>

#include "spatia/config.hpp"
#include "spatia/geometry/angle.hpp"

namespace spatia {

/// Two consecutive angles about the Z and Y axes of one system, used as a
/// two-angle parameterization of a direction.
template <class System>
class EulerZY {
    static_assert(dimension_v<System> == 3);

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
template <class From, class To>
class EulerZYX {
    static_assert(dimension_v<From> == 3 && dimension_v<To> == 3);

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
