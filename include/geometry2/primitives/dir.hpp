#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

#include "geometry2/primitives/fwd.hpp"
#include "geometry2/primitives/point.hpp"
#include "geometry2/primitives/vector.hpp"

namespace geometry2 {

template <class Frame>
Dir<Frame> dir_of(const Vector<Frame>& vector);

template <class Frame>
Dir<Frame> dir_to(const Point<Frame>& point);

template <class Frame>
constexpr const Vector<Frame>& as_vector(const Dir<Frame>& direction);

template <class Frame>
constexpr auto operator*(const Dir<Frame>& direction,
                         typename Frame::scalar_type length) -> Vector<Frame>;

template <class Frame>
constexpr auto operator*(typename Frame::scalar_type length,
                         const Dir<Frame>& direction) -> Vector<Frame>;

// Cartesian unit coordinates make rotations cheap and avoid the singularities
// of a minimal angular representation. The magnitude is canonicalized to one.
template <class Frame>
class Dir {
   public:
    using Scalar = typename Frame::scalar_type;

    constexpr const Scalar& operator[](std::size_t index) const noexcept;
    constexpr bool operator==(const Dir&) const;

   private:
    explicit constexpr Dir(Vector<Frame> unit);

    Vector<Frame> unit_;

    template <class F>
    friend Dir<F> dir_of(const Vector<F>&);

    template <class F>
    friend constexpr const Vector<F>& as_vector(const Dir<F>&);
};

template <class Frame>
    requires(Frame::dimension == 2)
using Dir2 = Dir<Frame>;

template <class Frame>
    requires(Frame::dimension == 3)
using Dir3 = Dir<Frame>;

// Implementation

template <class Frame>
constexpr Dir<Frame>::Dir(Vector<Frame> unit) : unit_(std::move(unit)) {}

template <class Frame>
constexpr const typename Dir<Frame>::Scalar& Dir<Frame>::operator[](
    std::size_t index) const noexcept {
    return unit_[index];
}

template <class Frame>
constexpr bool Dir<Frame>::operator==(const Dir&) const = default;

template <class Frame>
Dir<Frame> dir_of(const Vector<Frame>& vector) {
    const typename Frame::scalar_type length = math::norm(vector);
    if (!(length > typename Frame::scalar_type{})) {
        throw std::invalid_argument("Cannot create Dir from a zero vector");
    }
    return Dir<Frame>{vector / length};
}

template <class Frame>
Dir<Frame> dir_to(const Point<Frame>& point) {
    return dir_of(point - Point<Frame>{});
}

template <class Frame>
constexpr const Vector<Frame>& as_vector(const Dir<Frame>& direction) {
    return direction.unit_;
}

template <class Frame>
constexpr auto operator*(const Dir<Frame>& direction,
                         typename Frame::scalar_type length) -> Vector<Frame> {
    return as_vector(direction) * length;
}

template <class Frame>
constexpr auto operator*(typename Frame::scalar_type length,
                         const Dir<Frame>& direction) -> Vector<Frame> {
    return direction * length;
}

}  // namespace geometry2
