#pragma once

#include "spatia/transforms/rotation.hpp"
#include "spatia/transforms/translation.hpp"

namespace spatia {

/// Rotation followed by translation, mapping objects from `From` into `To` in
/// that direction only. Use `inverse()` for the opposite direction, or
/// `BiRigid` when a single object has to serve both directions.
template <class From, class To>
class Rigid {
    static_assert(dimension_v<From> == dimension_v<To>);
    static_assert(dimension_v<From> == 2 || dimension_v<From> == 3);

   public:
    static constexpr std::size_t dimension = dimension_v<From>;
    using FromSystem = From;
    using ToSystem = To;
    using Transformations = TransformList<TransformSpec<Point<To>, Point<From>>,
                                          TransformSpec<Vector<To>, Vector<From>>, TransformSpec<Dir<To>, Dir<From>>,
                                          TransformSpec<Line<To>, Line<From>>, TransformSpec<Line<To>, Dir<From>>>;

    constexpr Rigid() = default;
    /// Builds the transform from the typed parts it is made of.
    constexpr Rigid(Rotation<From, To> rotation, Translation<From, To> translation)
        : rotation_(rotation), translation_(translation.translation()) {}
    /// Builds the transform from raw algebra, bypassing the typed parts.
    constexpr Rigid(Matrix<dimension, dimension> rotation, Vec<dimension> translation)
        : rotation_(Rotation<From, To>{rotation}), translation_(Vector<To>::from_vec(translation)) {}

    constexpr const Rotation<From, To>& rotation() const noexcept { return rotation_; }
    constexpr const Vector<To>& translation() const noexcept { return translation_; }

    constexpr Point<To> operator()(const Point<From>& point, Tag<Point<To>> = {}) const {
        return rotation_.rotate_about_shared_origin(point) + translation_;
    }
    constexpr Vector<To> operator()(const Vector<From>& vector, Tag<Vector<To>> = {}) const {
        return rotation_(vector);
    }
    Dir<To> operator()(const Dir<From>& direction, Tag<Dir<To>> = {}) const { return rotation_(direction); }
    Line<To> operator()(const Line<From>& line, Tag<Line<To>> = {}) const;

    /// Deliberately without a default tag: a direction already maps to a
    /// `Dir` by default, so turning it into the line through the transformed
    /// origin has to be requested explicitly.
    Line<To> operator()(const Dir<From>& direction, Tag<Line<To>>) const;

    constexpr Rigid<To, From> inverse() const;

   private:
    Rotation<From, To> rotation_;
    Vector<To> translation_;
};

/// Rigid transform usable in both directions: it is a `Rigid<A, B>`
/// and a `Rigid<B, A>` at the same time, so the transform graph can
/// traverse it either way.
template <class A, class B>
class BiRigid : public Rigid<A, B>, public Rigid<B, A> {
   public:
    static constexpr std::size_t dimension = dimension_v<A>;
    using FromSystem = A;
    using ToSystem = B;
    using Transformations =
        TransformList<TransformSpec<Point<B>, Point<A>>, TransformSpec<Point<A>, Point<B>>,
                      TransformSpec<Vector<B>, Vector<A>>, TransformSpec<Vector<A>, Vector<B>>,
                      TransformSpec<Dir<B>, Dir<A>>, TransformSpec<Dir<A>, Dir<B>>, TransformSpec<Line<B>, Line<A>>,
                      TransformSpec<Line<A>, Line<B>>, TransformSpec<Line<B>, Dir<A>>, TransformSpec<Line<A>, Dir<B>>>;

    constexpr BiRigid() : BiRigid(Rigid<A, B>{}) {}
    constexpr BiRigid(Rotation<A, B> rotation, Translation<A, B> translation)
        : BiRigid(Rigid<A, B>{rotation, translation}) {}
    constexpr BiRigid(Matrix<dimension, dimension> rotation, Vec<dimension> translation)
        : BiRigid(Rigid<A, B>{rotation, translation}) {}
    constexpr BiRigid(Rigid<A, B> forward) : Rigid<A, B>(forward), Rigid<B, A>(forward.inverse()) {}

    using Rigid<A, B>::operator();
    using Rigid<B, A>::operator();
    using Rigid<A, B>::rotation;
    using Rigid<A, B>::translation;

    constexpr BiRigid<B, A> inverse() const { return BiRigid<B, A>{static_cast<const Rigid<B, A>&>(*this)}; }
};

// Implementation ======================================================================================================

template <class From, class To>
Line<To> Rigid<From, To>::operator()(const Line<From>& line, Tag<Line<To>>) const {
    return line_through((*this)(closest_point_to_origin(line)), (*this)(dir_of(line)));
}

template <class From, class To>
Line<To> Rigid<From, To>::operator()(const Dir<From>& direction, Tag<Line<To>>) const {
    return line_through(Point<To>{} + translation_, (*this)(direction));
}

template <class From, class To>
constexpr Rigid<To, From> Rigid<From, To>::inverse() const {
    const auto inverse_rotation = rotation_.inverse();
    return {inverse_rotation, Translation<To, From>{inverse_rotation(-translation_)}};
}

}  // namespace spatia
