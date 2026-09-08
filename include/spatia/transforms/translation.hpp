#pragma once

#include "spatia/geometry/line.hpp"
#include "spatia/transforms/detail/transform_graph_fwd.hpp"

namespace spatia {

/// Pure translation mapping objects from `From` into `To`, in that direction
/// only. Use `inverse()` for the opposite direction, or
/// `BiTranslation` when a single object has to serve both directions.
template <class From, class To>
class Translation {
    static_assert(dimension_v<From> == dimension_v<To>);

   public:
    using FromSystem = From;
    using ToSystem = To;
    using Transformations =
        TransformList<TransformSpec<Point<To>, Point<From>>, TransformSpec<Vector<To>, Vector<From>>,
                      TransformSpec<Dir<To>, Dir<From>>, TransformSpec<Line<To>, Line<From>>>;

    constexpr Translation() = default;
    constexpr explicit Translation(Vector<To> translation) : translation_(translation) {}
    /// The origin of `From` expressed in `To`: maps `Point<From>{}` to `from_origin_in_to`.
    constexpr explicit Translation(Point<To> from_origin_in_to) : Translation(from_origin_in_to - Point<To>{}) {}

    constexpr const Vector<To>& translation() const noexcept { return translation_; }

    constexpr Point<To> operator()(const Point<From>& point, Tag<Point<To>> = {}) const {
        return Point<To>{point.to_vec()} + translation_;
    }
    constexpr Vector<To> operator()(const Vector<From>& vector, Tag<Vector<To>> = {}) const {
        return Vector<To>{vector.to_vec()};
    }
    Dir<To> operator()(const Dir<From>& direction, Tag<Dir<To>> = {}) const {
        return Dir<To>::from_vec(to_vector(direction).to_vec());
    }
    Line<To> operator()(const Line<From>& line, Tag<Line<To>> = {}) const;

    constexpr Translation<To, From> inverse() const {
        return Translation<To, From>{Vector<From>{-translation_.to_vec()}};
    }

   private:
    Vector<To> translation_;
};

/// Translation usable in both directions: it is a
/// `Translation<A, B>` and a `Translation<B, A>` at the same
/// time, so the transform graph can traverse it either way.
template <class A, class B>
class BiTranslation : public Translation<A, B>, public Translation<B, A> {
   public:
    using FromSystem = A;
    using ToSystem = B;
    using Transformations = TransformList<TransformSpec<Point<B>, Point<A>>, TransformSpec<Point<A>, Point<B>>,
                                          TransformSpec<Vector<B>, Vector<A>>, TransformSpec<Vector<A>, Vector<B>>,
                                          TransformSpec<Dir<B>, Dir<A>>, TransformSpec<Dir<A>, Dir<B>>,
                                          TransformSpec<Line<B>, Line<A>>, TransformSpec<Line<A>, Line<B>>>;

    constexpr BiTranslation() : BiTranslation(Translation<A, B>{}) {}
    constexpr explicit BiTranslation(Vector<B> translation) : BiTranslation(Translation<A, B>{translation}) {}
    /// The origin of `A` expressed in `B`.
    constexpr explicit BiTranslation(Point<B> a_origin_in_b) : BiTranslation(Translation<A, B>{a_origin_in_b}) {}
    constexpr BiTranslation(Translation<A, B> forward)
        : Translation<A, B>(forward), Translation<B, A>(forward.inverse()) {}

    using Translation<A, B>::operator();
    using Translation<B, A>::operator();
    using Translation<A, B>::translation;

    constexpr BiTranslation<B, A> inverse() const {
        return BiTranslation<B, A>{static_cast<const Translation<B, A>&>(*this)};
    }
};

// Implementation ======================================================================================================

template <class From, class To>
Line<To> Translation<From, To>::operator()(const Line<From>& line, Tag<Line<To>>) const {
    return line_through((*this)(closest_point_to_origin(line)), (*this)(dir_of(line)));
}

}  // namespace spatia
