#pragma once

#include "geometry2/math/point.hpp"

namespace geometry2 {

template <class Frame>
using Point = math::Point<Frame::dimension, typename Frame::scalar_type, Frame>;

template <class Frame>
    requires(Frame::dimension == 2)
using Point2 = Point<Frame>;

template <class Frame>
    requires(Frame::dimension == 3)
using Point3 = Point<Frame>;

}  // namespace geometry2
