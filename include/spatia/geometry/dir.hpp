#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

#include "spatia/geometry/point.hpp"
#include "spatia/geometry/vector.hpp"

namespace spatia {

/// Normalized direction in a concrete coordinate system. Construction
/// normalizes; the coordinates are read through `to_vector()`, so the
/// invariant cannot be broken.
template <class System>
class Dir {
    static_assert(dimension_v<System> > 0);

   public:
    static constexpr std::size_t dimension = dimension_v<System>;

    static Dir from_vec(const Vec<dimension>& coordinates) { return dir_of(Vector<System>::from_vec(coordinates)); }
    static Dir from_vector(const Vector<System>& vector) { return dir_of(vector); }

    constexpr bool operator==(const Dir&) const = default;

   private:
    explicit constexpr Dir(Vector<System> unit) : unit_(std::move(unit)) {}

    Vector<System> unit_;

    template <class S>
    friend Dir<S> dir_of(const Vector<S>&);

    template <class S>
    friend constexpr const Vector<S>& to_vector(const Dir<S>&);
};

template <class System>
Dir<System> dir_of(const Vector<System>& vector);

template <class System>
Dir<System> dir_to(const Point<System>& point);

template <class System>
constexpr const Vector<System>& to_vector(const Dir<System>& direction);

template <class System>
constexpr Vector<System> operator*(const Dir<System>& direction, Scalar length);

template <class System>
constexpr Vector<System> operator*(Scalar length, const Dir<System>& direction);

// Implementation ======================================================================================================

template <class System>
Dir<System> dir_of(const Vector<System>& vector) {
    const Scalar length = norm(vector);
    if (!(length > Scalar{})) {
        throw std::invalid_argument("Cannot create Dir from a zero vector");
    }
    return Dir<System>{vector / length};
}

template <class System>
Dir<System> dir_to(const Point<System>& point) {
    return dir_of(point - Point<System>{});
}

template <class System>
constexpr const Vector<System>& to_vector(const Dir<System>& direction) {
    return direction.unit_;
}

template <class System>
constexpr Vector<System> operator*(const Dir<System>& direction, Scalar length) {
    return to_vector(direction) * length;
}

template <class System>
constexpr Vector<System> operator*(Scalar length, const Dir<System>& direction) {
    return direction * length;
}

}  // namespace spatia
