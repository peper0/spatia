#pragma once

#include <utility>

#include "geometry2/primitives/dir.hpp"
#include "geometry2/primitives/fwd.hpp"
#include "geometry2/primitives/point.hpp"

namespace geometry2 {

template <class Frame>
Line<Frame> line_through(const Point<Frame>& point,
                         const Dir<Frame>& direction);

template <class Frame>
Line<Frame> line_through(const Point<Frame>& first, const Point<Frame>& second);

template <class Frame>
Point<Frame> closest_point_to_origin(const Line<Frame>& line);

template <class Frame>
Dir<Frame> dir_of(const Line<Frame>& line);

// A line is stored canonically as its closest point to the coordinate-system
// origin plus a unit direction. There is no arbitrary anchor or direction
// scale.
template <class Frame>
class Line {
   public:
    static Line through(const Point<Frame>& point, const Dir<Frame>& direction);

    constexpr bool operator==(const Line&) const;

   private:
    constexpr Line(Point<Frame> closest, Dir<Frame> direction);

    Point<Frame> closest_;
    Dir<Frame> direction_;

    template <class F>
    friend Point<F> closest_point_to_origin(const Line<F>&);

    template <class F>
    friend Dir<F> dir_of(const Line<F>&);
};

template <class Frame>
    requires(Frame::dimension == 2)
using Line2 = Line<Frame>;

template <class Frame>
    requires(Frame::dimension == 3)
using Line3 = Line<Frame>;

// Implementation

template <class Frame>
Line<Frame> Line<Frame>::through(const Point<Frame>& point,
                                 const Dir<Frame>& direction) {
    const auto from_origin = point - Point<Frame>{};
    const auto closest =
        point - direction * math::dot(from_origin, as_vector(direction));
    return Line{closest, direction};
}

template <class Frame>
constexpr Line<Frame>::Line(Point<Frame> closest, Dir<Frame> direction)
    : closest_(std::move(closest)), direction_(std::move(direction)) {}

template <class Frame>
constexpr bool Line<Frame>::operator==(const Line&) const = default;

template <class Frame>
Line<Frame> line_through(const Point<Frame>& point,
                         const Dir<Frame>& direction) {
    return Line<Frame>::through(point, direction);
}

template <class Frame>
Line<Frame> line_through(const Point<Frame>& first,
                         const Point<Frame>& second) {
    return line_through(first, dir_of(second - first));
}

template <class Frame>
Point<Frame> closest_point_to_origin(const Line<Frame>& line) {
    return line.closest_;
}

template <class Frame>
Dir<Frame> dir_of(const Line<Frame>& line) {
    return line.direction_;
}

}  // namespace geometry2
