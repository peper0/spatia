#pragma once

#include "geometry2/math/matrix.hpp"
#include "geometry2/transforms/affine_transform2.hpp"
#include "geometry2/transforms/rigid_transform3.hpp"
#include "geometry2/transforms/rotation_transform3.hpp"

namespace geometry2 {

// compose(first, second) forms a left-to-right transformation chain:
// the returned transform is equivalent to second(first(value)).
template <class From, class Middle, class To>
RotationTransform3<From, To> compose(
    const RotationTransform3<From, Middle>& first,
    const RotationTransform3<Middle, To>& second);

template <class From, class Middle, class To>
RigidTransform3<From, To> compose(const RotationTransform3<From, Middle>& first,
                                  const RigidTransform3<Middle, To>& second);

template <class From, class Middle, class To>
RigidTransform3<From, To> compose(const RigidTransform3<From, Middle>& first,
                                  const RotationTransform3<Middle, To>& second);

template <class From, class Middle, class To>
RigidTransform3<From, To> compose(const RigidTransform3<From, Middle>& first,
                                  const RigidTransform3<Middle, To>& second);

template <class From, class Middle, class To>
AffineTransform2<From, To> compose(const AffineTransform2<From, Middle>& first,
                                   const AffineTransform2<Middle, To>& second);

// Implementation

template <class From, class Middle, class To>
RotationTransform3<From, To> compose(
    const RotationTransform3<From, Middle>& first,
    const RotationTransform3<Middle, To>& second) {
    return RotationTransform3<From, To>{second.rotation_matrix() *
                                        first.rotation_matrix()};
}

template <class From, class Middle, class To>
RigidTransform3<From, To> compose(const RotationTransform3<From, Middle>& first,
                                  const RigidTransform3<Middle, To>& second) {
    return RigidTransform3<From, To>{
        second.rotation_matrix() * first.rotation_matrix(),
        second.translation()};
}

template <class From, class Middle, class To>
RigidTransform3<From, To> compose(
    const RigidTransform3<From, Middle>& first,
    const RotationTransform3<Middle, To>& second) {
    return RigidTransform3<From, To>{
        second.rotation_matrix() * first.rotation_matrix(),
        math::apply_matrix<typename To::Vector>(second.rotation_matrix(),
                                                first.translation())};
}

template <class From, class Middle, class To>
RigidTransform3<From, To> compose(const RigidTransform3<From, Middle>& first,
                                  const RigidTransform3<Middle, To>& second) {
    return RigidTransform3<From, To>{
        second.rotation_matrix() * first.rotation_matrix(),
        math::apply_matrix<typename To::Vector>(second.rotation_matrix(),
                                                first.translation()) +
            second.translation()};
}

template <class From, class Middle, class To>
AffineTransform2<From, To> compose(const AffineTransform2<From, Middle>& first,
                                   const AffineTransform2<Middle, To>& second) {
    return AffineTransform2<From, To>{
        second.linear_matrix() * first.linear_matrix(),
        math::apply_matrix<typename To::Vector>(second.linear_matrix(),
                                                first.translation()) +
            second.translation()};
}

}  // namespace geometry2
