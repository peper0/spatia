#pragma once

#include <utility>

#include "spatia/geometry/dir.hpp"
#include "spatia/geometry/point.hpp"

namespace spatia {

/// Directed line stored in canonical form: the point of the line closest to
/// the origin plus the direction.
template <class System>
class Line {
   public:
    static Line through(const Point<System>& point, const Dir<System>& direction);

    constexpr bool operator==(const Line& other) const;

   private:
    constexpr Line(Point<System> closest, Dir<System> direction)
        : closest_(std::move(closest)), direction_(std::move(direction)) {}

    Point<System> closest_;
    Dir<System> direction_;

    template <class S>
    friend Point<S> closest_point_to_origin(const Line<S>&);

    template <class S>
    friend Dir<S> dir_of(const Line<S>&);
};

template <class System>
Line<System> line_through(const Point<System>& point, const Dir<System>& direction);

template <class System>
Line<System> line_through(const Point<System>& first, const Point<System>& second);

template <class System>
Point<System> closest_point_to_origin(const Line<System>& line);

template <class System>
Dir<System> dir_of(const Line<System>& line);

// Implementation ======================================================================================================

template <class System>
Line<System> Line<System>::through(const Point<System>& point, const Dir<System>& direction) {
    const auto from_origin = point - Point<System>{};
    const auto closest = point - direction * dot(from_origin, to_vector(direction));
    return Line{closest, direction};
}

template <class System>
constexpr bool Line<System>::operator==(const Line& other) const {
    // Both objects are canonical, but building the same directed line from
    // different anchors leaves floating-point rounding differences, so the
    // comparison asks "do the two objects describe the same line" within a
    // relative tolerance instead of comparing members exactly.
    constexpr Scalar tolerance = Scalar{1e-12};
    const auto direction_difference = to_vector(direction_) - to_vector(other.direction_);
    const auto offset = closest_ - other.closest_;
    const auto from_origin = closest_ - Point<System>{};
    const Scalar point_scale = Scalar{1} + dot(from_origin, from_origin);
    return dot(direction_difference, direction_difference) <= tolerance * tolerance &&
           dot(offset, offset) <= tolerance * tolerance * point_scale;
}

template <class System>
Line<System> line_through(const Point<System>& point, const Dir<System>& direction) {
    return Line<System>::through(point, direction);
}

template <class System>
Line<System> line_through(const Point<System>& first, const Point<System>& second) {
    return line_through(first, dir_of(second - first));
}

template <class System>
Point<System> closest_point_to_origin(const Line<System>& line) {
    return line.closest_;
}

template <class System>
Dir<System> dir_of(const Line<System>& line) {
    return line.direction_;
}

}  // namespace spatia
