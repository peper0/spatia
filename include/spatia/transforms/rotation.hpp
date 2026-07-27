#pragma once

#include "spatia/algebra/matrix.hpp"
#include "spatia/algebra/quaternion.hpp"
#include "spatia/geometry/line.hpp"
#include "spatia/transforms/detail/quaternion_matrix.hpp"
#include "spatia/transforms/detail/transform_graph_fwd.hpp"

namespace spatia {

/// Pure rotation mapping objects from `From` into `To`, in that direction
/// only. Use `inverse()` for the opposite direction, or `BiRotation` when a
/// single object has to serve both directions. Works in two dimensions (a
/// rotation in the plane) and in three.
template <class From, class To>
class Rotation {
    static_assert(dimension_v<From> == dimension_v<To>);
    static_assert(dimension_v<From> == 2 || dimension_v<From> == 3);

   public:
    static constexpr std::size_t dimension = dimension_v<From>;
    using FromSystem = From;
    using ToSystem = To;
    using Transformations =
        TransformList<TransformSpec<Point<To>, Point<From>>, TransformSpec<Vector<To>, Vector<From>>,
                      TransformSpec<Dir<To>, Dir<From>>, TransformSpec<Line<To>, Line<From>>>;

    constexpr Rotation() : matrix_(identity_matrix<dimension>()) {}
    constexpr explicit Rotation(Matrix<dimension, dimension> matrix) : matrix_(matrix) {}

    constexpr const Matrix<dimension, dimension>& to_matrix() const noexcept { return matrix_; }
    Quaternion to_quaternion() const
        requires(dimension == 3)
    {
        return detail::rotation_matrix_quaternion(matrix_);
    }

    // The destination tag defaults, so each conversion is a single overload
    // that both plain calls and the transform graph can use.
    constexpr Vector<To> operator()(const Vector<From>& vector, Tag<Vector<To>> = {}) const {
        return Vector<To>::from_vec(matrix_ * vector.to_vec());
    }
    Dir<To> operator()(const Dir<From>& direction, Tag<Dir<To>> = {}) const {
        return Dir<To>::from_vector((*this)(to_vector(direction)));
    }
    Line<To> operator()(const Line<From>& line, Tag<Line<To>> = {}) const;

    constexpr Point<To> rotate_about_shared_origin(const Point<From>& point) const {
        return Point<To>::from_vec(matrix_ * point.to_vec());
    }

    /// Deliberately without a default tag: rotating a point assumes that the
    /// two systems share an origin, and that assumption has to be spelled out
    /// — either by `rotate_about_shared_origin` or by naming the destination.
    constexpr Point<To> operator()(const Point<From>& point, Tag<Point<To>>) const {
        return rotate_about_shared_origin(point);
    }

    constexpr Rotation<To, From> inverse() const { return Rotation<To, From>{transposed(matrix_)}; }

   private:
    Matrix<dimension, dimension> matrix_;
};

/// Rotation usable in both directions: it is a `Rotation<A, B>` and a
/// `Rotation<B, A>` at the same time, so the transform graph can traverse it
/// either way.
template <class A, class B>
class BiRotation : public Rotation<A, B>, public Rotation<B, A> {
   public:
    static constexpr std::size_t dimension = dimension_v<A>;
    using FromSystem = A;
    using ToSystem = B;
    using Transformations = TransformList<TransformSpec<Point<B>, Point<A>>, TransformSpec<Point<A>, Point<B>>,
                                          TransformSpec<Vector<B>, Vector<A>>, TransformSpec<Vector<A>, Vector<B>>,
                                          TransformSpec<Dir<B>, Dir<A>>, TransformSpec<Dir<A>, Dir<B>>,
                                          TransformSpec<Line<B>, Line<A>>, TransformSpec<Line<A>, Line<B>>>;

    constexpr BiRotation() = default;
    constexpr explicit BiRotation(Matrix<dimension, dimension> matrix)
        : Rotation<A, B>(matrix), Rotation<B, A>(transposed(matrix)) {}
    constexpr BiRotation(Rotation<A, B> forward) : Rotation<A, B>(forward), Rotation<B, A>(forward.inverse()) {}

    using Rotation<A, B>::operator();
    using Rotation<B, A>::operator();
    using Rotation<A, B>::rotate_about_shared_origin;
    using Rotation<B, A>::rotate_about_shared_origin;
    using Rotation<A, B>::to_matrix;
    using Rotation<A, B>::to_quaternion;

    constexpr BiRotation<B, A> inverse() const { return BiRotation<B, A>{static_cast<const Rotation<B, A>&>(*this)}; }
};

// Implementation ======================================================================================================

template <class From, class To>
Line<To> Rotation<From, To>::operator()(const Line<From>& line, Tag<Line<To>>) const {
    return line_through(rotate_about_shared_origin(closest_point_to_origin(line)), (*this)(dir_of(line)));
}

}  // namespace spatia
