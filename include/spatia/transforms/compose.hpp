#pragma once

#include <utility>

#include "spatia/transforms/affine.hpp"
#include "spatia/transforms/conversions.hpp"
#include "spatia/transforms/detail/transform_traits.hpp"
#include "spatia/transforms/perspective_projection.hpp"
#include "spatia/transforms/projective.hpp"
#include "spatia/transforms/rigid.hpp"
#include "spatia/transforms/rotation.hpp"
#include "spatia/transforms/translation.hpp"

namespace spatia {

/// Composition of two transforms with one conversion each: it converts the
/// source of `first` into the destination of `second`. Transform kinds that
/// can be composed into a single object of the same kind have their own
/// `operator*` overloads below, which are more specialized and therefore win.
template <class First, class Second>
class ComposedPair {
    using FirstSpec = detail::sole_spec_of<First>;
    using SecondSpec = detail::sole_spec_of<Second>;
    static_assert(std::is_same_v<typename FirstSpec::Dst, typename SecondSpec::Src>,
                  "the destination of the first transform must be the source of the second one");

   public:
    using Source = typename FirstSpec::Src;
    using Destination = typename SecondSpec::Dst;
    using Transformations = TransformList<TransformSpec<Destination, Source>>;

    constexpr ComposedPair(First first, Second second) : first_(std::move(first)), second_(std::move(second)) {}

    Destination operator()(const Source& source, Tag<Destination> = {}) const { return second_(first_(source)); }

   private:
    First first_;
    Second second_;
};

/// Composes two arbitrary single-conversion transforms, following the matrix
/// convention: `bc * ab` applies `ab` first.
template <class Second, class First>
    requires SingleConversionTransform<std::decay_t<Second>> && SingleConversionTransform<std::decay_t<First>>
ComposedPair<std::decay_t<First>, std::decay_t<Second>> operator*(Second&& bc, First&& ab) {
    return ComposedPair<std::decay_t<First>, std::decay_t<Second>>(std::forward<First>(ab), std::forward<Second>(bc));
}

// Rotation
template <class A, class B, class C>
constexpr Rotation<A, C> operator*(const Rotation<B, C>& bc, const Rotation<A, B>& ab);

// Translation
template <class A, class B, class C>
constexpr Translation<A, C> operator*(const Translation<B, C>& bc, const Translation<A, B>& ab);

// Rigid, Rotation, Translation
template <class A, class B, class C>
constexpr Rigid<A, C> operator*(const Rigid<B, C>& bc, const Rigid<A, B>& ab);

template <class A, class B, class C>
constexpr Rigid<A, C> operator*(const Rigid<B, C>& bc, const Rotation<A, B>& ab);

template <class A, class B, class C>
constexpr Rigid<A, C> operator*(const Rotation<B, C>& bc, const Rigid<A, B>& ab);

template <class A, class B, class C>
constexpr Rigid<A, C> operator*(const Rigid<B, C>& bc, const Translation<A, B>& ab);

template <class A, class B, class C>
constexpr Rigid<A, C> operator*(const Translation<B, C>& bc, const Rigid<A, B>& ab);

template <class A, class B, class C>
constexpr Rigid<A, C> operator*(const Translation<B, C>& bc, const Rotation<A, B>& ab);

template <class A, class B, class C>
constexpr Rigid<A, C> operator*(const Rotation<B, C>& bc, const Translation<A, B>& ab);

// Affine
template <class A, class B, class C>
constexpr Affine<A, C> operator*(const Affine<B, C>& bc, const Affine<A, B>& ab);

// Projective
//
// Projective transforms are the closure of all the kinds above under
// composition, so any product involving one — including a perspective
// projection, which is projective but not affine — lands here.
template <class A, class B, class C>
Projective<A, C> operator*(const Projective<B, C>& bc, const Projective<A, B>& ab);

template <class A, class B, class C>
Projective<A, C> operator*(const Projective<B, C>& bc, const Affine<A, B>& ab);

template <class A, class B, class C>
Projective<A, C> operator*(const Projective<B, C>& bc, const Rigid<A, B>& ab);

template <class A, class B, class C>
Projective<A, C> operator*(const Projective<B, C>& bc, const Rotation<A, B>& ab);

template <class A, class B, class C>
Projective<A, C> operator*(const Projective<B, C>& bc, const Translation<A, B>& ab);

template <class A, class B, class C>
Projective<A, C> operator*(const Affine<B, C>& bc, const Projective<A, B>& ab);

template <class A, class B, class C>
Projective<A, C> operator*(const Rigid<B, C>& bc, const Projective<A, B>& ab);

template <class A, class B, class C>
Projective<A, C> operator*(const Rotation<B, C>& bc, const Projective<A, B>& ab);

template <class A, class B, class C>
Projective<A, C> operator*(const Translation<B, C>& bc, const Projective<A, B>& ab);

template <class A, class B, class Plane>
Projective<A, Plane> operator*(const PerspectiveProjection<B, Plane>& bc, const Projective<A, B>& ab);

template <class A, class B, class Plane>
Projective<A, Plane> operator*(const PerspectiveProjection<B, Plane>& bc, const Affine<A, B>& ab);

template <class A, class B, class Plane>
Projective<A, Plane> operator*(const PerspectiveProjection<B, Plane>& bc, const Rigid<A, B>& ab);

template <class A, class B, class Plane>
Projective<A, Plane> operator*(const PerspectiveProjection<B, Plane>& bc, const Rotation<A, B>& ab);

template <class A, class B, class Plane>
Projective<A, Plane> operator*(const PerspectiveProjection<B, Plane>& bc, const Translation<A, B>& ab);

// The bidirectional overloads are needed because template argument deduction
// against a base with two `Rotation` specializations is ambiguous.
template <class A, class B, class C>
constexpr BiRotation<A, C> operator*(const BiRotation<B, C>& bc, const BiRotation<A, B>& ab);

template <class A, class B, class C>
constexpr BiRigid<A, C> operator*(const BiRigid<B, C>& bc, const BiRigid<A, B>& ab);

template <class A, class B, class C>
BiAffine<A, C> operator*(const BiAffine<B, C>& bc, const BiAffine<A, B>& ab);

template <class A, class B, class C>
constexpr BiTranslation<A, C> operator*(const BiTranslation<B, C>& bc, const BiTranslation<A, B>& ab);

// Implementation ======================================================================================================

template <class A, class B, class C>
constexpr Rotation<A, C> operator*(const Rotation<B, C>& bc, const Rotation<A, B>& ab) {
    return Rotation<A, C>{bc.to_matrix() * ab.to_matrix()};
}

template <class A, class B, class C>
constexpr Rigid<A, C> operator*(const Rigid<B, C>& bc, const Rigid<A, B>& ab) {
    return {bc.rotation() * ab.rotation(), Translation<A, C>{bc.rotation()(ab.translation()) + bc.translation()}};
}

template <class A, class B, class C>
constexpr Rigid<A, C> operator*(const Rigid<B, C>& bc, const Rotation<A, B>& ab) {
    return {bc.rotation() * ab, Translation<A, C>{bc.translation()}};
}

template <class A, class B, class C>
constexpr Rigid<A, C> operator*(const Rotation<B, C>& bc, const Rigid<A, B>& ab) {
    return {bc * ab.rotation(), Translation<A, C>{bc(ab.translation())}};
}

template <class A, class B, class C>
constexpr Affine<A, C> operator*(const Affine<B, C>& bc, const Affine<A, B>& ab) {
    return {bc.linear() * ab.linear(), Vector<C>::from_vec(bc.linear() * ab.translation().to_vec()) + bc.translation()};
}

template <class A, class B, class C>
constexpr Translation<A, C> operator*(const Translation<B, C>& bc, const Translation<A, B>& ab) {
    return Translation<A, C>{Vector<C>::from_vec(ab.translation().to_vec()) + bc.translation()};
}

// A translation leaves the axes alone, so in the mixed products below the
// rotation matrix carries over unchanged and only the offsets combine.

template <class A, class B, class C>
constexpr Rigid<A, C> operator*(const Rigid<B, C>& bc, const Translation<A, B>& ab) {
    return {Rotation<A, C>{bc.rotation().to_matrix()},
            Translation<A, C>{bc.rotation()(ab.translation()) + bc.translation()}};
}

template <class A, class B, class C>
constexpr Rigid<A, C> operator*(const Translation<B, C>& bc, const Rigid<A, B>& ab) {
    return {Rotation<A, C>{ab.rotation().to_matrix()},
            Translation<A, C>{Vector<C>::from_vec(ab.translation().to_vec()) + bc.translation()}};
}

template <class A, class B, class C>
constexpr Rigid<A, C> operator*(const Translation<B, C>& bc, const Rotation<A, B>& ab) {
    return {Rotation<A, C>{ab.to_matrix()}, Translation<A, C>{bc.translation()}};
}

template <class A, class B, class C>
constexpr Rigid<A, C> operator*(const Rotation<B, C>& bc, const Translation<A, B>& ab) {
    return {Rotation<A, C>{bc.to_matrix()}, Translation<A, C>{bc(ab.translation())}};
}

// Every product below reduces to multiplying the homogeneous matrices, so the
// operands are first widened into their projective form.

template <class A, class B, class C>
Projective<A, C> operator*(const Projective<B, C>& bc, const Projective<A, B>& ab) {
    return Projective<A, C>{bc.to_matrix() * ab.to_matrix()};
}

template <class A, class B, class C>
Projective<A, C> operator*(const Projective<B, C>& bc, const Affine<A, B>& ab) {
    return bc * to_projective(ab);
}

template <class A, class B, class C>
Projective<A, C> operator*(const Projective<B, C>& bc, const Rigid<A, B>& ab) {
    return bc * to_projective(ab);
}

template <class A, class B, class C>
Projective<A, C> operator*(const Projective<B, C>& bc, const Rotation<A, B>& ab) {
    return bc * to_projective(ab);
}

template <class A, class B, class C>
Projective<A, C> operator*(const Projective<B, C>& bc, const Translation<A, B>& ab) {
    return bc * to_projective(ab);
}

template <class A, class B, class C>
Projective<A, C> operator*(const Affine<B, C>& bc, const Projective<A, B>& ab) {
    return to_projective(bc) * ab;
}

template <class A, class B, class C>
Projective<A, C> operator*(const Rigid<B, C>& bc, const Projective<A, B>& ab) {
    return to_projective(bc) * ab;
}

template <class A, class B, class C>
Projective<A, C> operator*(const Rotation<B, C>& bc, const Projective<A, B>& ab) {
    return to_projective(bc) * ab;
}

template <class A, class B, class C>
Projective<A, C> operator*(const Translation<B, C>& bc, const Projective<A, B>& ab) {
    return to_projective(bc) * ab;
}

template <class A, class B, class Plane>
Projective<A, Plane> operator*(const PerspectiveProjection<B, Plane>& bc, const Projective<A, B>& ab) {
    return to_projective(bc) * ab;
}

template <class A, class B, class Plane>
Projective<A, Plane> operator*(const PerspectiveProjection<B, Plane>& bc, const Affine<A, B>& ab) {
    return to_projective(bc) * to_projective(ab);
}

template <class A, class B, class Plane>
Projective<A, Plane> operator*(const PerspectiveProjection<B, Plane>& bc, const Rigid<A, B>& ab) {
    return to_projective(bc) * to_projective(ab);
}

template <class A, class B, class Plane>
Projective<A, Plane> operator*(const PerspectiveProjection<B, Plane>& bc, const Rotation<A, B>& ab) {
    return to_projective(bc) * to_projective(ab);
}

template <class A, class B, class Plane>
Projective<A, Plane> operator*(const PerspectiveProjection<B, Plane>& bc, const Translation<A, B>& ab) {
    return to_projective(bc) * to_projective(ab);
}

template <class A, class B, class C>
constexpr BiRotation<A, C> operator*(const BiRotation<B, C>& bc, const BiRotation<A, B>& ab) {
    return static_cast<const Rotation<B, C>&>(bc) * static_cast<const Rotation<A, B>&>(ab);
}

template <class A, class B, class C>
constexpr BiRigid<A, C> operator*(const BiRigid<B, C>& bc, const BiRigid<A, B>& ab) {
    return static_cast<const Rigid<B, C>&>(bc) * static_cast<const Rigid<A, B>&>(ab);
}

template <class A, class B, class C>
BiAffine<A, C> operator*(const BiAffine<B, C>& bc, const BiAffine<A, B>& ab) {
    return static_cast<const Affine<B, C>&>(bc) * static_cast<const Affine<A, B>&>(ab);
}

template <class A, class B, class C>
constexpr BiTranslation<A, C> operator*(const BiTranslation<B, C>& bc, const BiTranslation<A, B>& ab) {
    return static_cast<const Translation<B, C>&>(bc) * static_cast<const Translation<A, B>&>(ab);
}

}  // namespace spatia
