#pragma once

#include "spatia/algebra/matrix.hpp"
#include "spatia/geometry/line.hpp"
#include "spatia/transforms/detail/transform_graph_fwd.hpp"
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

    constexpr Rigid() : rotation_(identity_matrix<dimension>()) {}
    constexpr Rigid(Matrix<dimension, dimension> rotation, Point<To> from_origin_in_to)
        : rotation_(rotation), from_origin_in_to_(from_origin_in_to) {}

    template <class Tmp>
    constexpr Rigid(Rotation<Tmp, To> rotation, Translation<From, Tmp> translation)
        : Rigid(rotation.to_matrix(), rotation(translation(Point<From>{}))) {}

    constexpr const Matrix<dimension, dimension>& rotation() const noexcept { return rotation_; }
    constexpr const Point<To>& from_origin_in_to() const noexcept { return from_origin_in_to_; }

    constexpr Point<To> operator()(const Point<From>& point, Tag<Point<To>> = {}) const {
        return from_origin_in_to_ + (*this)(point - Point<From>{});
    }
    constexpr Vector<To> operator()(const Vector<From>& vector, Tag<Vector<To>> = {}) const {
        return Vector<To>{rotation_ * vector.to_vec()};
    }
    Dir<To> operator()(const Dir<From>& direction, Tag<Dir<To>> = {}) const {
        return Dir<To>::from_vector((*this)(to_vector(direction)));
    }
    Line<To> operator()(const Line<From>& line, Tag<Line<To>> = {}) const;

    /// Deliberately without a default tag: a direction already maps to a
    /// `Dir` by default, so turning it into the line through the transformed
    /// origin has to be requested explicitly.
    Line<To> operator()(const Dir<From>& direction, Tag<Line<To>>) const;

    constexpr Rigid<To, From> inverse() const;

   private:
    Matrix<dimension, dimension> rotation_;
    Point<To> from_origin_in_to_;
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
    constexpr BiRigid(Matrix<dimension, dimension> rotation, Point<B> a_origin_in_b)
        : BiRigid(Rigid<A, B>{rotation, a_origin_in_b}) {}

    template <class Tmp>
    constexpr BiRigid(Rotation<Tmp, B> rotation, Translation<A, Tmp> translation)
        : BiRigid(Rigid<A, B>{rotation, translation}) {}

    constexpr BiRigid(Rigid<A, B> forward) : Rigid<A, B>(forward), Rigid<B, A>(forward.inverse()) {}

    using Rigid<A, B>::operator();
    using Rigid<B, A>::operator();
    using Rigid<A, B>::rotation;
    using Rigid<A, B>::from_origin_in_to;

    constexpr BiRigid<B, A> inverse() const { return BiRigid<B, A>{static_cast<const Rigid<B, A>&>(*this)}; }
};

// Implementation ======================================================================================================

template <class From, class To>
Line<To> Rigid<From, To>::operator()(const Line<From>& line, Tag<Line<To>>) const {
    return line_through((*this)(closest_point_to_origin(line)), (*this)(dir_of(line)));
}

template <class From, class To>
Line<To> Rigid<From, To>::operator()(const Dir<From>& direction, Tag<Line<To>>) const {
    return line_through(from_origin_in_to_, (*this)(direction));
}

template <class From, class To>
constexpr Rigid<To, From> Rigid<From, To>::inverse() const {
    const auto inverse_rotation = transposed(rotation_);
    return {inverse_rotation, Point<From>{inverse_rotation * (-from_origin_in_to_.to_vec())}};
}

}  // namespace spatia
