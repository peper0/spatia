#pragma once

#include <cstddef>

#include "spatia/geometry/dir.hpp"
#include "spatia/geometry/line.hpp"
#include "spatia/geometry/point.hpp"
#include "spatia/geometry/vector.hpp"

namespace spatia {

/// Optional facade for declaring a coordinate system together with aliases of
/// the geometric types expressed in it. Deriving from it is equivalent to
/// declaring only `static constexpr std::size_t dimension`.
template <class Derived, std::size_t Dimension>
struct CoordinateSystem {
    static constexpr std::size_t dimension = Dimension;
    using Point = spatia::Point<Derived>;
    using Vector = spatia::Vector<Derived>;
    using Dir = spatia::Dir<Derived>;
    using Line = spatia::Line<Derived>;
};

}  // namespace spatia
