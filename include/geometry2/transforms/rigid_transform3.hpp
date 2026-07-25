#pragma once

#include <concepts>

#include "compose.hpp"
#include "geometry2/math/matrix.hpp"
#include "geometry2/primitives/line.hpp"
#include "geometry2/transforms/rotation_transform3.hpp"

namespace geometry2 {

template <class From, class To>
class RigidTransform3 {
    static_assert(From::dimension == 3 && To::dimension == 3);
    static_assert(
        std::same_as<typename From::scalar_type, typename To::scalar_type>);

   public:
    using Scalar = typename To::scalar_type;
    using transformations = transform_list<
        transform_spec<typename To::Point, typename From::Point>,
        transform_spec<typename From::Point, typename To::Point>,
        transform_spec<typename To::Vector, typename From::Vector>,
        transform_spec<typename From::Vector, typename To::Vector>,
        transform_spec<typename To::Dir, typename From::Dir>,
        transform_spec<typename From::Dir, typename To::Dir>,
        transform_spec<typename To::Line, typename From::Line>,
        transform_spec<typename From::Line, typename To::Line>,
        transform_spec<typename To::Line, typename From::Dir>,
        transform_spec<typename From::Line, typename To::Dir>>;

    RigidTransform3(math::SquareMatrix<3, Scalar> rotation,
                    typename To::Vector translation);

    const math::SquareMatrix<3, Scalar>& rotation_matrix() const noexcept;
    const typename To::Vector& translation() const noexcept;

    typename To::Point operator()(const typename From::Point& point,
                                  Tag<typename To::Point>) const;
    typename From::Point operator()(const typename To::Point& point,
                                    Tag<typename From::Point>) const;
    typename To::Vector operator()(const typename From::Vector& vector,
                                   Tag<typename To::Vector>) const;
    typename From::Vector operator()(const typename To::Vector& vector,
                                     Tag<typename From::Vector>) const;
    typename To::Dir operator()(const typename From::Dir& direction,
                                Tag<typename To::Dir>) const;
    typename From::Dir operator()(const typename To::Dir& direction,
                                  Tag<typename From::Dir>) const;
    typename To::Line operator()(const typename From::Line& line,
                                 Tag<typename To::Line>) const;
    typename From::Line operator()(const typename To::Line& line,
                                   Tag<typename From::Line>) const;
    typename To::Line operator()(const typename From::Dir& direction,
                                 Tag<typename To::Line>) const;
    typename From::Line operator()(const typename To::Dir& direction,
                                   Tag<typename From::Line>) const;

   private:
    RotationTransform3<From, To> rotation_;
    typename To::Vector translation_;
};

// Implementation

template <class From, class To>
RigidTransform3<From, To>::RigidTransform3(
    math::SquareMatrix<3, Scalar> rotation, typename To::Vector translation)
    : rotation_(rotation), translation_(translation) {}

template <class From, class To>
const math::SquareMatrix<3, typename RigidTransform3<From, To>::Scalar>&
RigidTransform3<From, To>::rotation_matrix() const noexcept {
    return rotation_.rotation_matrix();
}

template <class From, class To>
const typename To::Vector& RigidTransform3<From, To>::translation()
    const noexcept {
    return translation_;
}

template <class From, class To>
typename To::Point RigidTransform3<From, To>::operator()(
    const typename From::Point& point, Tag<typename To::Point>) const {
    return rotation_(point, Tag<typename To::Point>{}) + translation_;
}

template <class From, class To>
typename From::Point RigidTransform3<From, To>::operator()(
    const typename To::Point& point, Tag<typename From::Point>) const {
    return rotation_(point - translation_, Tag<typename From::Point>{});
}

template <class From, class To>
typename To::Vector RigidTransform3<From, To>::operator()(
    const typename From::Vector& vector, Tag<typename To::Vector>) const {
    return rotation_(vector, Tag<typename To::Vector>{});
}

template <class From, class To>
typename From::Vector RigidTransform3<From, To>::operator()(
    const typename To::Vector& vector, Tag<typename From::Vector>) const {
    return rotation_(vector, Tag<typename From::Vector>{});
}

template <class From, class To>
typename To::Dir RigidTransform3<From, To>::operator()(
    const typename From::Dir& direction, Tag<typename To::Dir>) const {
    return rotation_(direction, Tag<typename To::Dir>{});
}

template <class From, class To>
typename From::Dir RigidTransform3<From, To>::operator()(
    const typename To::Dir& direction, Tag<typename From::Dir>) const {
    return rotation_(direction, Tag<typename From::Dir>{});
}

template <class From, class To>
typename To::Line RigidTransform3<From, To>::operator()(
    const typename From::Line& line, Tag<typename To::Line>) const {
    return line_through(
        (*this)(closest_point_to_origin(line), Tag<typename To::Point>{}),
        (*this)(dir_of(line), Tag<typename To::Dir>{}));
}

template <class From, class To>
typename From::Line RigidTransform3<From, To>::operator()(
    const typename To::Line& line, Tag<typename From::Line>) const {
    return line_through(
        (*this)(closest_point_to_origin(line), Tag<typename From::Point>{}),
        (*this)(dir_of(line), Tag<typename From::Dir>{}));
}

// A Dir has no position. When requested as a Line, it is interpreted as the
// directed line through the source coordinate-system origin.
template <class From, class To>
typename To::Line RigidTransform3<From, To>::operator()(
    const typename From::Dir& direction, Tag<typename To::Line>) const {
    return line_through(typename To::Point{} + translation_,
                        (*this)(direction, Tag<typename To::Dir>{}));
}

template <class From, class To>
typename From::Line RigidTransform3<From, To>::operator()(
    const typename To::Dir& direction, Tag<typename From::Line>) const {
    return line_through(
        (*this)(typename To::Point{}, Tag<typename From::Point>{}),
        (*this)(direction, Tag<typename From::Dir>{}));
}

}  // namespace geometry2
