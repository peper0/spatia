#pragma once

#include <cstddef>

namespace spatia {

/// Scalar used by the public spatia API.
using Scalar = double;

/// Dimension carried by a coordinate-system type.
template <class System>
inline constexpr std::size_t dimension_v = System::dimension;

}  // namespace spatia
