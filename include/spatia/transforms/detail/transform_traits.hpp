#pragma once

#include <type_traits>

#include "spatia/transforms/detail/transform_graph_fwd.hpp"

namespace spatia::detail {

// Extracts a TransformSpec from a call signature: the return type is the
// destination, the first parameter is the source. Extra parameters (Tag) are
// ignored.
template <class F>
struct FnTraits;

template <class R, class Arg, class... Rest>
struct FnTraits<R(Arg, Rest...)> {
    using Spec = TransformSpec<R, std::remove_cvref_t<Arg>>;
};

template <class R, class Arg, class... Rest>
struct FnTraits<R(Arg, Rest...) const> : FnTraits<R(Arg, Rest...)> {};

template <class R, class Arg, class... Rest>
struct FnTraits<R(Arg, Rest...) noexcept> : FnTraits<R(Arg, Rest...)> {};

template <class R, class Arg, class... Rest>
struct FnTraits<R(Arg, Rest...) const noexcept> : FnTraits<R(Arg, Rest...)> {};

template <class F>
struct FnTraits<F*> : FnTraits<F> {};

template <class C, class F>
struct FnTraits<F C::*> : FnTraits<F> {};

// Signature inference: function pointers directly, classes through their sole
// non-template operator().
template <class T, class = void>
struct InferredSpecs {
    using Type = TransformList<typename FnTraits<decltype(&T::operator())>::Spec>;
};

template <class T>
struct InferredSpecs<T, std::enable_if_t<std::is_pointer_v<T>>> {
    using Type = TransformList<typename FnTraits<T>::Spec>;
};

// An explicit `Transformations` typedef takes precedence over inference.
template <class T, class = void>
struct SpecsOf : InferredSpecs<T> {};

template <class T>
struct SpecsOf<T, std::void_t<typename T::Transformations>> {
    using Type = typename T::Transformations;
};

// The single spec of a transform with exactly one conversion edge.
template <class T>
struct SoleSpec {
    static_assert(sizeof(T) == 0,
                  "this transform provides more than one conversion, so the "
                  "composition is ambiguous: use combine() instead, or one of "
                  "the operator* overloads for a specific transform kind");
};

template <class Spec>
struct SoleSpec<TransformList<Spec>> {
    using Type = Spec;
};

template <class T>
using sole_spec_of = typename SoleSpec<typename SpecsOf<T>::Type>::Type;

template <class List>
struct SpecCount : std::integral_constant<std::size_t, 0> {};

template <class... Specs>
struct SpecCount<TransformList<Specs...>> : std::integral_constant<std::size_t, sizeof...(Specs)> {};

// How many conversions T declares, without ever hard-erroring: types that are
// not transforms at all simply report zero. Written with `if constexpr` rather
// than partial specializations because a `Transformations` typedef and a sole
// `operator()` can both be present, which would make the specializations
// ambiguous.
template <class T>
consteval std::size_t conversion_count() {
    if constexpr (requires { typename T::Transformations; }) {
        return SpecCount<typename T::Transformations>::value;
    } else if constexpr (std::is_pointer_v<T> && std::is_function_v<std::remove_pointer_t<T>>) {
        return 1;
    } else if constexpr (requires { &T::operator(); }) {
        // Taking the address succeeds only for a single non-template overload.
        return 1;
    } else {
        return 0;
    }
}

}  // namespace spatia::detail

namespace spatia {

/// A transform providing exactly one conversion, so that composing it with
/// another one is unambiguous. Plain functions and non-generic lambdas
/// qualify; `Rotation` and the other multi-conversion transforms do not.
template <class T>
concept SingleConversionTransform = detail::conversion_count<T>() == 1;

}  // namespace spatia

namespace spatia::detail {}  // namespace spatia::detail
