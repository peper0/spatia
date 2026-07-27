#pragma once

#include "spatia/algebra/matrix.hpp"
#include "spatia/geometry/line.hpp"
#include "spatia/transforms/detail/transform_graph_fwd.hpp"

namespace spatia {

/// General linear map plus translation, mapping objects from `From` into `To`
/// in that direction only. Use `inverse()` for the opposite direction, or
/// `BiAffine` when a single object has to serve both directions.
template <class From, class To>
class Affine {
    static_assert(dimension_v<From> == dimension_v<To>);
    static_assert(dimension_v<From> == 2 || dimension_v<From> == 3);

   public:
    static constexpr std::size_t dimension = dimension_v<From>;
    using FromSystem = From;
    using ToSystem = To;
    using Transformations =
        TransformList<TransformSpec<Point<To>, Point<From>>, TransformSpec<Vector<To>, Vector<From>>,
                      TransformSpec<Dir<To>, Dir<From>>, TransformSpec<Line<To>, Line<From>>>;

    constexpr Affine() : linear_(identity_matrix<dimension>()) {}
    constexpr Affine(Matrix<dimension, dimension> linear, Vector<To> translation)
        : linear_(linear), translation_(translation) {}

    constexpr const Matrix<dimension, dimension>& linear() const noexcept { return linear_; }
    constexpr const Vector<To>& translation() const noexcept { return translation_; }

    constexpr Point<To> operator()(const Point<From>& point, Tag<Point<To>> = {}) const {
        return Point<To>::from_vec(linear_ * point.to_vec()) + translation_;
    }
    constexpr Vector<To> operator()(const Vector<From>& vector, Tag<Vector<To>> = {}) const {
        return Vector<To>::from_vec(linear_ * vector.to_vec());
    }
    Dir<To> operator()(const Dir<From>& direction, Tag<Dir<To>> = {}) const {
        return Dir<To>::from_vector((*this)(to_vector(direction)));
    }
    Line<To> operator()(const Line<From>& line, Tag<Line<To>> = {}) const;

    Affine<To, From> inverse() const;

   private:
    Matrix<dimension, dimension> linear_;
    Vector<To> translation_;
};

/// Affine transform usable in both directions: it is an
/// `Affine<A, B>` and an `Affine<B, A>` at the same time, so
/// the transform graph can traverse it either way. Construction inverts the
/// linear part eagerly, so it requires an invertible transform.
template <class A, class B>
class BiAffine : public Affine<A, B>, public Affine<B, A> {
   public:
    static constexpr std::size_t dimension = dimension_v<A>;
    using FromSystem = A;
    using ToSystem = B;
    using Transformations = TransformList<TransformSpec<Point<B>, Point<A>>, TransformSpec<Point<A>, Point<B>>,
                                          TransformSpec<Vector<B>, Vector<A>>, TransformSpec<Vector<A>, Vector<B>>,
                                          TransformSpec<Dir<B>, Dir<A>>, TransformSpec<Dir<A>, Dir<B>>,
                                          TransformSpec<Line<B>, Line<A>>, TransformSpec<Line<A>, Line<B>>>;

    BiAffine() : BiAffine(Affine<A, B>{}) {}
    BiAffine(Matrix<dimension, dimension> linear, Vector<B> translation)
        : BiAffine(Affine<A, B>{linear, translation}) {}
    BiAffine(Affine<A, B> forward) : Affine<A, B>(forward), Affine<B, A>(forward.inverse()) {}

    using Affine<A, B>::operator();
    using Affine<B, A>::operator();
    using Affine<A, B>::linear;
    using Affine<A, B>::translation;

    BiAffine<B, A> inverse() const { return BiAffine<B, A>{static_cast<const Affine<B, A>&>(*this)}; }
};

// Implementation ======================================================================================================

template <class From, class To>
Line<To> Affine<From, To>::operator()(const Line<From>& line, Tag<Line<To>>) const {
    return line_through((*this)(closest_point_to_origin(line)), (*this)(dir_of(line)));
}

template <class From, class To>
Affine<To, From> Affine<From, To>::inverse() const {
    const auto inverse_linear = spatia::inverse(linear_);
    return {inverse_linear, Vector<From>::from_vec(inverse_linear * (-translation_).to_vec())};
}

}  // namespace spatia
