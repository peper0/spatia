#pragma once

#include <cmath>
#include <limits>
#include <stdexcept>

#include "spatia/geometry/dir.hpp"
#include "spatia/geometry/point.hpp"
#include "spatia/transforms/detail/transform_graph_fwd.hpp"

namespace spatia {

template <class FromPlane, class ToSpace>
class PerspectiveUnprojection;

/// Central perspective projection of a three-dimensional system onto the
/// two-dimensional system of a projection surface: a point is divided by its
/// third coordinate, then scaled and offset. The classic use is a camera and
/// its image, but nothing here is camera-specific.
///
/// The projection direction is `FromSpace -> To` only, because projecting
/// discards the distance along the projection axis. `PerspectiveUnprojection`
/// travels the other way and recovers a direction instead of a point;
/// `BiPerspectiveProjection` bundles both.
template <class From, class To>
class PerspectiveProjection {
    static_assert(dimension_v<From> == 3 && dimension_v<To> == 2);

   public:
    using FromSystem = From;
    using ToSystem = To;
    using Transformations = TransformList<TransformSpec<Point<To>, Point<From>>, TransformSpec<Point<To>, Dir<From>>>;

    /// `scale_x` and `scale_y` are the projection distances measured in the
    /// units of `To`; `center_x` and `center_y` locate the projection
    /// axis on the plane.
    PerspectiveProjection(Scalar scale_x, Scalar scale_y, Scalar center_x, Scalar center_y);

    // The destination tag defaults, so each conversion is a single overload
    // that both plain calls and the transform graph can use.
    Point<To> operator()(const Point<From>& point, Tag<Point<To>> = {}) const {
        return project(point.x(), point.y(), point.z());
    }
    Point<To> operator()(const Dir<From>& direction, Tag<Point<To>> = {}) const {
        const Vector<From>& unit = to_vector(direction);
        return project(unit.dx(), unit.dy(), unit.dz());
    }

    constexpr Scalar scale_x() const noexcept { return scale_x_; }
    constexpr Scalar scale_y() const noexcept { return scale_y_; }
    constexpr Scalar center_x() const noexcept { return center_x_; }
    constexpr Scalar center_y() const noexcept { return center_y_; }

    /// The opposite direction recovers only the direction towards the
    /// projected point, never the point itself.
    PerspectiveUnprojection<To, From> inverse() const;

   private:
    Point<To> project(Scalar x, Scalar y, Scalar z) const;

    Scalar scale_x_;
    Scalar scale_y_;
    Scalar center_x_;
    Scalar center_y_;
};

/// The `FromPlane -> ToSpace` direction of a perspective projection: a point
/// of the plane names the direction in space that projects onto it. The
/// distance is unrecoverable, which is why the result is a `Dir`.
template <class FromPlane, class ToSpace>
class PerspectiveUnprojection {
    static_assert(dimension_v<FromPlane> == 2 && dimension_v<ToSpace> == 3);

   public:
    using FromSystem = FromPlane;
    using ToSystem = ToSpace;
    using Transformations = TransformList<TransformSpec<Dir<ToSpace>, Point<FromPlane>>>;

    PerspectiveUnprojection(Scalar scale_x, Scalar scale_y, Scalar center_x, Scalar center_y);

    Dir<ToSpace> operator()(const Point<FromPlane>& point, Tag<Dir<ToSpace>> = {}) const;

    constexpr Scalar scale_x() const noexcept { return scale_x_; }
    constexpr Scalar scale_y() const noexcept { return scale_y_; }
    constexpr Scalar center_x() const noexcept { return center_x_; }
    constexpr Scalar center_y() const noexcept { return center_y_; }

    PerspectiveProjection<ToSpace, FromPlane> inverse() const;

   private:
    Scalar scale_x_;
    Scalar scale_y_;
    Scalar center_x_;
    Scalar center_y_;
    // Precomputed because unprojecting divides by the scales on every call.
    Scalar inverse_scale_x_;
    Scalar inverse_scale_y_;
};

/// A perspective projection usable in both directions: it is a
/// `PerspectiveProjection<Space, Plane>` and a
/// `PerspectiveUnprojection<Plane, Space>` at the same time, so the transform
/// graph can traverse it either way.
template <class Space, class Plane>
class BiPerspectiveProjection : public PerspectiveProjection<Space, Plane>,
                                public PerspectiveUnprojection<Plane, Space> {
   public:
    using FromSystem = Space;
    using ToSystem = Plane;
    using Transformations =
        TransformList<TransformSpec<Point<Plane>, Point<Space>>, TransformSpec<Point<Plane>, Dir<Space>>,
                      TransformSpec<Dir<Space>, Point<Plane>>>;

    BiPerspectiveProjection(Scalar scale_x, Scalar scale_y, Scalar center_x, Scalar center_y)
        : PerspectiveProjection<Space, Plane>(scale_x, scale_y, center_x, center_y),
          PerspectiveUnprojection<Plane, Space>(scale_x, scale_y, center_x, center_y) {}

    using PerspectiveProjection<Space, Plane>::operator();
    using PerspectiveUnprojection<Plane, Space>::operator();
    using PerspectiveProjection<Space, Plane>::scale_x;
    using PerspectiveProjection<Space, Plane>::scale_y;
    using PerspectiveProjection<Space, Plane>::center_x;
    using PerspectiveProjection<Space, Plane>::center_y;

    BiPerspectiveProjection<Plane, Space> inverse() const = delete;
};

// Implementation ======================================================================================================

template <class FromSpace, class ToPlane>
PerspectiveProjection<FromSpace, ToPlane>::PerspectiveProjection(Scalar scale_x, Scalar scale_y, Scalar center_x,
                                                                 Scalar center_y)
    : scale_x_(scale_x), scale_y_(scale_y), center_x_(center_x), center_y_(center_y) {
    if (scale_x == Scalar{} || scale_y == Scalar{}) {
        throw std::invalid_argument("Perspective projection scale cannot be zero");
    }
}

template <class FromSpace, class ToPlane>
Point<ToPlane> PerspectiveProjection<FromSpace, ToPlane>::project(Scalar x, Scalar y, Scalar z) const {
    if (std::abs(z) <= std::numeric_limits<Scalar>::epsilon()) {
        throw std::domain_error("Cannot project a point lying in the plane through the projection centre");
    }
    // One reciprocal instead of two divisions. Nothing more can be hoisted
    // into the object: `z` only becomes known here, so a stored `scale_x / z`
    // is not possible.
    const Scalar inverse_z = Scalar{1} / z;
    return Point<ToPlane>{scale_x_ * x * inverse_z + center_x_, scale_y_ * y * inverse_z + center_y_};
}

template <class FromSpace, class ToPlane>
PerspectiveUnprojection<ToPlane, FromSpace> PerspectiveProjection<FromSpace, ToPlane>::inverse() const {
    return {scale_x_, scale_y_, center_x_, center_y_};
}

template <class FromPlane, class ToSpace>
PerspectiveUnprojection<FromPlane, ToSpace>::PerspectiveUnprojection(Scalar scale_x, Scalar scale_y, Scalar center_x,
                                                                     Scalar center_y)
    : scale_x_(scale_x),
      scale_y_(scale_y),
      center_x_(center_x),
      center_y_(center_y),
      inverse_scale_x_(Scalar{1} / scale_x),
      inverse_scale_y_(Scalar{1} / scale_y) {
    if (scale_x == Scalar{} || scale_y == Scalar{}) {
        throw std::invalid_argument("Perspective projection scale cannot be zero");
    }
}

template <class FromPlane, class ToSpace>
Dir<ToSpace> PerspectiveUnprojection<FromPlane, ToSpace>::operator()(const Point<FromPlane>& point,
                                                                     Tag<Dir<ToSpace>>) const {
    // The reciprocals of the scales are stored, so unprojecting needs no
    // division at all. Both forms are kept: the accessors report the scales
    // the caller passed in, without a lossy round trip through 1/x.
    return dir_of(Vector<ToSpace>{(point[0] - center_x_) * inverse_scale_x_, (point[1] - center_y_) * inverse_scale_y_,
                                  Scalar{1}});
}

template <class FromPlane, class ToSpace>
PerspectiveProjection<ToSpace, FromPlane> PerspectiveUnprojection<FromPlane, ToSpace>::inverse() const {
    return {scale_x_, scale_y_, center_x_, center_y_};
}

}  // namespace spatia
