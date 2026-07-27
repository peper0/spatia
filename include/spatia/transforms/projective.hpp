#pragma once

#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>

#include "spatia/algebra/matrix.hpp"
#include "spatia/algebra/matrix_utils.hpp"
#include "spatia/geometry/line.hpp"
#include "spatia/geometry/point.hpp"
#include "spatia/transforms/detail/transform_graph_fwd.hpp"

namespace spatia {

/// Projective transform: the most general mapping of points that keeps
/// straight lines straight. It stores a homogeneous matrix with one extra row
/// and column, so a 3D-to-3D transform holds a 4x4 homography and a
/// 3D-to-2D projection holds a 3x4 one. Applying it multiplies the
/// homogeneous coordinates and divides by the resulting last coordinate.
///
/// This is the closure of the other transform kinds under composition: an
/// affine transform composed with a perspective projection is no longer
/// affine, but it is still projective, so every such product lands here.
/// Vectors and directions are deliberately absent: a projective transform
/// does not act on them consistently, because the division depends on the
/// point at which it is evaluated.
template <class From, class To>
class Projective {
    static_assert(dimension_v<From> >= 2 && dimension_v<To> >= 2);

   public:
    static constexpr std::size_t from_dimension = dimension_v<From>;
    static constexpr std::size_t to_dimension = dimension_v<To>;
    using FromSystem = From;
    using ToSystem = To;
    using HomogeneousMatrix = Matrix<to_dimension + 1, from_dimension + 1>;
    using Transformations = TransformList<TransformSpec<Point<To>, Point<From>>, TransformSpec<Line<To>, Line<From>>>;

    constexpr Projective() : matrix_(identity_homography()) {}
    constexpr explicit Projective(HomogeneousMatrix matrix) : matrix_(matrix) {}

    constexpr const HomogeneousMatrix& to_matrix() const noexcept { return matrix_; }

    Point<To> operator()(const Point<From>& point, Tag<Point<To>> = {}) const;

    /// Keeping straight lines straight is what makes a transform projective,
    /// so lines map to lines. A line through the centre of projection has no
    /// image and is rejected.
    Line<To> operator()(const Line<From>& line, Tag<Line<To>> = {}) const;

    /// Only available when the transform maps between systems of the same
    /// dimension, that is when the homography is square.
    Projective<To, From> inverse() const
        requires(from_dimension == to_dimension);

   private:
    static constexpr HomogeneousMatrix identity_homography();

    HomogeneousMatrix matrix_;
};

// Implementation ======================================================================================================

template <class From, class To>
constexpr typename Projective<From, To>::HomogeneousMatrix Projective<From, To>::identity_homography() {
    HomogeneousMatrix result;
    for (std::size_t i = 0; i < to_dimension + 1 && i < from_dimension + 1; ++i) {
        result(i, i) = Scalar{1};
    }
    return result;
}

template <class From, class To>
Point<To> Projective<From, To>::operator()(const Point<From>& point, Tag<Point<To>>) const {
    Vec<from_dimension + 1> homogeneous;
    for (std::size_t i = 0; i < from_dimension; ++i) {
        homogeneous[i] = point[i];
    }
    homogeneous[from_dimension] = Scalar{1};

    const Vec<to_dimension + 1> projected = matrix_ * homogeneous;
    const Scalar weight = projected[to_dimension];
    if (std::abs(weight) <= std::numeric_limits<Scalar>::epsilon()) {
        throw std::domain_error("Projective transform maps this point to infinity");
    }

    Vec<to_dimension> result;
    for (std::size_t i = 0; i < to_dimension; ++i) {
        result[i] = projected[i] / weight;
    }
    return Point<To>::from_vec(result);
}

template <class From, class To>
Line<To> Projective<From, To>::operator()(const Line<From>& line, Tag<Line<To>>) const {
    // Two distinct points determine the image line, because the transform
    // cannot bend it. `line_through` rejects the degenerate case where both
    // points collapse onto one.
    const Point<From> anchor = closest_point_to_origin(line);
    const Point<From> ahead = anchor + dir_of(line) * Scalar{1};
    return line_through((*this)(anchor), (*this)(ahead));
}

template <class From, class To>
Projective<To, From> Projective<From, To>::inverse() const
    requires(from_dimension == to_dimension)
{
    return Projective<To, From>{spatia::inverse(matrix_)};
}

}  // namespace spatia
