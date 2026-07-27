#pragma once

// Lightweight declarations of the transform-graph vocabulary. Canonical
// transform headers only need these types; the full graph engine lives in
// "spatia/transforms/compose.hpp".

namespace spatia {

/// Selects the destination type of an overloaded transform call.
template <class T>
struct Tag {};

/// One directed conversion edge: `Destination` from `Source`.
template <class Destination, class Source>
struct TransformSpec {
    using Dst = Destination;
    using Src = Source;
};

/// The set of edges a single callable exposes, declared as a member typedef
/// `Transformations` on callables whose signature cannot be inferred.
template <class... Specifications>
struct TransformList {};

}  // namespace spatia
