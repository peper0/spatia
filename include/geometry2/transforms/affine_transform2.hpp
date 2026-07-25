#pragma once

#include <concepts>

#include "compose.hpp"
#include "geometry2/math/matrix.hpp"
#include "geometry2/primitives/line.hpp"

namespace geometry2 {

template <class From, class To>
class AffineTransform2 {
    static_assert(From::dimension == 2 && To::dimension == 2);
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
        transform_spec<typename From::Line, typename To::Line>>;

    AffineTransform2(math::SquareMatrix<2, Scalar> matrix,
                     typename To::Vector translation);

    const math::SquareMatrix<2, Scalar>& linear_matrix() const noexcept;
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

   private:
    math::SquareMatrix<2, Scalar> matrix_;
    math::SquareMatrix<2, Scalar> inverse_;
    typename To::Vector translation_;
};

// Implementation

template <class From, class To>
AffineTransform2<From, To>::AffineTransform2(
    math::SquareMatrix<2, Scalar> matrix, typename To::Vector translation)
    : matrix_(matrix),
      inverse_(math::inverse(matrix)),
      translation_(translation) {}

template <class From, class To>
const math::SquareMatrix<2, typename AffineTransform2<From, To>::Scalar>&
AffineTransform2<From, To>::linear_matrix() const noexcept {
    return matrix_;
}

template <class From, class To>
const typename To::Vector& AffineTransform2<From, To>::translation()
    const noexcept {
    return translation_;
}

template <class From, class To>
typename To::Point AffineTransform2<From, To>::operator()(
    const typename From::Point& point, Tag<typename To::Point>) const {
    return math::apply_matrix<typename To::Point>(matrix_, point) +
           translation_;
}

template <class From, class To>
typename From::Point AffineTransform2<From, To>::operator()(
    const typename To::Point& point, Tag<typename From::Point>) const {
    return math::apply_matrix<typename From::Point>(inverse_,
                                                    point - translation_);
}

template <class From, class To>
typename To::Vector AffineTransform2<From, To>::operator()(
    const typename From::Vector& vector, Tag<typename To::Vector>) const {
    return math::apply_matrix<typename To::Vector>(matrix_, vector);
}

template <class From, class To>
typename From::Vector AffineTransform2<From, To>::operator()(
    const typename To::Vector& vector, Tag<typename From::Vector>) const {
    return math::apply_matrix<typename From::Vector>(inverse_, vector);
}

template <class From, class To>
typename To::Dir AffineTransform2<From, To>::operator()(
    const typename From::Dir& direction, Tag<typename To::Dir>) const {
    return dir_of((*this)(as_vector(direction), Tag<typename To::Vector>{}));
}

template <class From, class To>
typename From::Dir AffineTransform2<From, To>::operator()(
    const typename To::Dir& direction, Tag<typename From::Dir>) const {
    return dir_of((*this)(as_vector(direction), Tag<typename From::Vector>{}));
}

template <class From, class To>
typename To::Line AffineTransform2<From, To>::operator()(
    const typename From::Line& line, Tag<typename To::Line>) const {
    return line_through(
        (*this)(closest_point_to_origin(line), Tag<typename To::Point>{}),
        (*this)(dir_of(line), Tag<typename To::Dir>{}));
}

template <class From, class To>
typename From::Line AffineTransform2<From, To>::operator()(
    const typename To::Line& line, Tag<typename From::Line>) const {
    return line_through(
        (*this)(closest_point_to_origin(line), Tag<typename From::Point>{}),
        (*this)(dir_of(line), Tag<typename From::Dir>{}));
}

}  // namespace geometry2
