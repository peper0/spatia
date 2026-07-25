#pragma once

#include <cmath>
#include <concepts>
#include <limits>
#include <stdexcept>

#include "compose.hpp"
#include "geometry2/primitives/dir.hpp"
#include "geometry2/primitives/point.hpp"

namespace geometry2 {

template <class Picture, class Camera>
class PinholeProjectionTransform {
    static_assert(Picture::dimension == 2 && Camera::dimension == 3);
    static_assert(std::same_as<typename Picture::scalar_type,
                               typename Camera::scalar_type>);

   public:
    using Scalar = typename Picture::scalar_type;
    using transformations = transform_list<
        transform_spec<typename Camera::Dir, typename Picture::Point>,
        transform_spec<typename Picture::Point, typename Camera::Dir>,
        transform_spec<typename Picture::Point, typename Camera::Point>>;

    PinholeProjectionTransform(Scalar focal_x, Scalar focal_y, Scalar center_x,
                               Scalar center_y);

    typename Camera::Dir operator()(const typename Picture::Point& point,
                                    Tag<typename Camera::Dir>) const;
    typename Picture::Point operator()(const typename Camera::Dir& direction,
                                       Tag<typename Picture::Point>) const;
    typename Picture::Point operator()(const typename Camera::Point& point,
                                       Tag<typename Picture::Point>) const;

   private:
    typename Picture::Point project(Scalar x, Scalar y, Scalar z) const;

    Scalar focal_x_;
    Scalar focal_y_;
    Scalar center_x_;
    Scalar center_y_;
};

// Implementation

template <class Picture, class Camera>
PinholeProjectionTransform<Picture, Camera>::PinholeProjectionTransform(
    Scalar focal_x, Scalar focal_y, Scalar center_x, Scalar center_y)
    : focal_x_(focal_x),
      focal_y_(focal_y),
      center_x_(center_x),
      center_y_(center_y) {
    if (focal_x == Scalar{} || focal_y == Scalar{}) {
        throw std::invalid_argument("Pinhole focal length cannot be zero");
    }
}

template <class Picture, class Camera>
typename Camera::Dir PinholeProjectionTransform<Picture, Camera>::operator()(
    const typename Picture::Point& point, Tag<typename Camera::Dir>) const {
    return dir_of(typename Camera::Vector{(point[0] - center_x_) / focal_x_,
                                          (point[1] - center_y_) / focal_y_,
                                          Scalar{1}});
}

template <class Picture, class Camera>
typename Picture::Point PinholeProjectionTransform<Picture, Camera>::operator()(
    const typename Camera::Dir& direction, Tag<typename Picture::Point>) const {
    return project(direction[0], direction[1], direction[2]);
}

template <class Picture, class Camera>
typename Picture::Point PinholeProjectionTransform<Picture, Camera>::operator()(
    const typename Camera::Point& point, Tag<typename Picture::Point>) const {
    return project(point[0], point[1], point[2]);
}

template <class Picture, class Camera>
typename Picture::Point PinholeProjectionTransform<Picture, Camera>::project(
    Scalar x, Scalar y, Scalar z) const {
    if (std::abs(z) <= std::numeric_limits<Scalar>::epsilon()) {
        throw std::domain_error(
            "Cannot project onto the camera plane when z is zero");
    }
    return typename Picture::Point{focal_x_ * x / z + center_x_,
                                   focal_y_ * y / z + center_y_};
}

}  // namespace geometry2
