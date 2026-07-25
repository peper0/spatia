#pragma once

#include <concepts>
#include <cstddef>

#include "geometry2/math/point.hpp"
#include "geometry2/math/vector.hpp"
#include "geometry2/primitives/dir.hpp"
#include "geometry2/primitives/line.hpp"

namespace geometry2 {

template <class Derived, std::size_t Dimension, class Scalar = double>
struct CoordinateSystem {
    static_assert(std::floating_point<Scalar>);

    static constexpr std::size_t dimension = Dimension;
    using scalar_type = Scalar;
    using Point = math::Point<Dimension, Scalar, Derived>;
    using Vector = math::Vector<Dimension, Scalar, Derived>;
    using Dir = geometry2::Dir<Derived>;
    using Line = geometry2::Line<Derived>;
};

}  // namespace geometry2
