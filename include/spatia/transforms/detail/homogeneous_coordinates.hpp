#pragma once

#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>

#include "spatia/algebra/vec.hpp"

namespace spatia::detail {

template <std::size_t Dimension>
constexpr Vec<Dimension + 1> to_homogeneous(const Vec<Dimension>& coordinates);

template <std::size_t HomogeneousDimension>
    requires(HomogeneousDimension > 1)
Vec<HomogeneousDimension - 1> from_homogeneous(const Vec<HomogeneousDimension>& homogeneous);

// Implementation ======================================================================================================

template <std::size_t Dimension>
constexpr Vec<Dimension + 1> to_homogeneous(const Vec<Dimension>& coordinates) {
    Vec<Dimension + 1> result;
    for (std::size_t i = 0; i < Dimension; ++i) {
        result[i] = coordinates[i];
    }
    result[Dimension] = Scalar{1};
    return result;
}

template <std::size_t HomogeneousDimension>
    requires(HomogeneousDimension > 1)
Vec<HomogeneousDimension - 1> from_homogeneous(const Vec<HomogeneousDimension>& homogeneous) {
    const Scalar weight = homogeneous[HomogeneousDimension - 1];
    if (std::abs(weight) <= std::numeric_limits<Scalar>::epsilon()) {
        throw std::domain_error("Projective transform maps this point to infinity");
    }

    Vec<HomogeneousDimension - 1> result;
    for (std::size_t i = 0; i < HomogeneousDimension - 1; ++i) {
        result[i] = homogeneous[i] / weight;
    }
    return result;
}

}  // namespace spatia::detail
