#pragma once

#include "geometry2/math/vector.hpp"

namespace geometry2 {

template <class Frame>
using Vector =
    math::Vector<Frame::dimension, typename Frame::scalar_type, Frame>;

template <class Frame>
    requires(Frame::dimension == 2)
using Vector2 = Vector<Frame>;

template <class Frame>
    requires(Frame::dimension == 3)
using Vector3 = Vector<Frame>;

}  // namespace geometry2
