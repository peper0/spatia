#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

// The tag selecting the destination type of an overloaded transform call.
// Only declared here; the program using this header provides the definition.
template <typename C>
struct Tag;

// One directed conversion edge: DST from SRC.
template <class DST, class SRC>
struct transform_spec {
    using dst = DST;
    using src = SRC;
};

// The set of edges a single callable exposes, declared as a member typedef
// `transformations` on callables whose signature cannot be inferred.
template <class... SPECS>
struct transform_list {};

// Selects one overload of an ordinary function by its exact signature.
template <class SIGNATURE>
constexpr SIGNATURE* overload_cast(SIGNATURE* fn) noexcept {
    return fn;
}

namespace compose_detail {

// Extracts a transform_spec from a call signature: the return type is the
// destination, the first parameter is the source. Extra parameters (Tag) are
// ignored.
template <class F>
struct fn_traits;

template <class R, class ARG, class... REST>
struct fn_traits<R(ARG, REST...)> {
    using spec = transform_spec<R, std::remove_cvref_t<ARG>>;
};

template <class R, class ARG, class... REST>
struct fn_traits<R(ARG, REST...) const> : fn_traits<R(ARG, REST...)> {};

template <class R, class ARG, class... REST>
struct fn_traits<R(ARG, REST...) noexcept> : fn_traits<R(ARG, REST...)> {};

template <class R, class ARG, class... REST>
struct fn_traits<R(ARG, REST...) const noexcept> : fn_traits<R(ARG, REST...)> {};

template <class F>
struct fn_traits<F*> : fn_traits<F> {};

template <class C, class F>
struct fn_traits<F C::*> : fn_traits<F> {};

// Signature inference: function pointers directly, classes through their sole
// non-template operator().
template <class T, class = void>
struct inferred_specs {
    using type = transform_list<typename fn_traits<decltype(&T::operator())>::spec>;
};

template <class T>
struct inferred_specs<T, std::enable_if_t<std::is_pointer_v<T>>> {
    using type = transform_list<typename fn_traits<T>::spec>;
};

// An explicit `transformations` typedef takes precedence over inference.
template <class T, class = void>
struct specs_of : inferred_specs<T> {};

template <class T>
struct specs_of<T, std::void_t<typename T::transformations>> {
    using type = typename T::transformations;
};

// A conversion edge bound to the transform stored at INDEX in the tuple.
template <std::size_t INDEX, class SRC, class DST>
struct edge {
    static constexpr std::size_t index = INDEX;
    using src = SRC;
    using dst = DST;
};

template <class... EDGES>
struct edge_list {};

template <class... LISTS>
struct concat;

template <>
struct concat<> {
    using type = edge_list<>;
};

template <class LIST>
struct concat<LIST> {
    using type = LIST;
};

template <class... AS, class... BS, class... REST>
struct concat<edge_list<AS...>, edge_list<BS...>, REST...>
    : concat<edge_list<AS..., BS...>, REST...> {};

template <std::size_t INDEX, class SPEC_LIST>
struct edges_of_one;

template <std::size_t INDEX, class... SPECS>
struct edges_of_one<INDEX, transform_list<SPECS...>> {
    using type = edge_list<edge<INDEX, typename SPECS::src, typename SPECS::dst>...>;
};

template <class SEQ, class... TS>
struct edges_of_all;

template <std::size_t... IS, class... TS>
struct edges_of_all<std::index_sequence<IS...>, TS...> {
    using type =
        typename concat<typename edges_of_one<IS, typename specs_of<TS>::type>::type...>::type;
};

template <class... TS>
struct type_list {};

template <class T, class LIST>
struct list_contains;

template <class T, class... TS>
struct list_contains<T, type_list<TS...>>
    : std::bool_constant<(std::is_same_v<T, TS> || ...)> {};

template <class T, class LIST>
struct list_push;

template <class T, class... TS>
struct list_push<T, type_list<TS...>> {
    using type = type_list<TS..., T>;
};

// Depth-first search over the edge graph. Any found path is correct as long
// as the composed transforms are mutually consistent; shortest-path order is
// not required.
struct no_path {};

template <class E, class PATH>
struct path_push_front;

template <class E, class... ES>
struct path_push_front<E, edge_list<ES...>> {
    using type = edge_list<E, ES...>;
};

template <class FROM, class TO, class ALL_EDGES, class VISITED>
struct find_path;

template <class FROM, class TO, class ALL_EDGES, class VISITED, class CANDIDATES>
struct search_edges;

template <class FROM, class TO, class ALL_EDGES, class VISITED>
struct search_edges<FROM, TO, ALL_EDGES, VISITED, edge_list<>> {
    using type = no_path;
};

// Lazily picks between "edge E followed by SUB" and the fallback search, so
// that the branch not taken is never instantiated.
template <class SUB, class E, class FALLBACK>
struct pick_path {
    using type = typename path_push_front<E, SUB>::type;
};

template <class E, class FALLBACK>
struct pick_path<no_path, E, FALLBACK> {
    using type = typename FALLBACK::type;
};

template <bool USABLE, class FROM, class TO, class ALL_EDGES, class VISITED, class E,
          class REST>
struct search_branch;

template <class FROM, class TO, class ALL_EDGES, class VISITED, class E, class REST>
struct search_branch<false, FROM, TO, ALL_EDGES, VISITED, E, REST>
    : search_edges<FROM, TO, ALL_EDGES, VISITED, REST> {};

template <class FROM, class TO, class ALL_EDGES, class VISITED, class E, class REST>
struct search_branch<true, FROM, TO, ALL_EDGES, VISITED, E, REST> {
    using sub = typename find_path<typename E::dst, TO, ALL_EDGES,
                                   typename list_push<typename E::dst, VISITED>::type>::type;
    using type =
        typename pick_path<sub, E, search_edges<FROM, TO, ALL_EDGES, VISITED, REST>>::type;
};

template <class FROM, class TO, class ALL_EDGES, class VISITED, class E, class... REST>
struct search_edges<FROM, TO, ALL_EDGES, VISITED, edge_list<E, REST...>>
    : search_branch<std::is_same_v<typename E::src, FROM> &&
                        !list_contains<typename E::dst, VISITED>::value,
                    FROM, TO, ALL_EDGES, VISITED, E, edge_list<REST...>> {};

template <class FROM, class TO, class ALL_EDGES, class VISITED>
struct find_path : search_edges<FROM, TO, ALL_EDGES, VISITED, ALL_EDGES> {};

template <class T, class ALL_EDGES, class VISITED>
struct find_path<T, T, ALL_EDGES, VISITED> {
    using type = edge_list<>;
};

// Calls one transform, preferring the tagged form so that overloaded
// callables resolve to the requested destination.
template <class DST, class T, class SRC>
DST apply_one(const T& t, const SRC& p) {
    if constexpr (std::is_invocable_v<const T&, const SRC&, Tag<DST>>) {
        return t(p, Tag<DST>{});
    } else {
        return t(p);
    }
}

template <class PATH>
struct path_runner;

template <>
struct path_runner<edge_list<>> {
    template <class TUPLE, class P>
    static const P& run(const TUPLE&, const P& p) {
        return p;
    }
};

template <class E, class... REST>
struct path_runner<edge_list<E, REST...>> {
    template <class TUPLE, class P>
    static auto run(const TUPLE& transforms, const P& p) {
        return path_runner<edge_list<REST...>>::run(
            transforms, apply_one<typename E::dst>(std::get<E::index>(transforms), p));
    }
};

}  // namespace compose_detail

template <class... TRANSFORMS>
class Composed {
public:
    explicit Composed(TRANSFORMS... transforms) : transforms_(std::move(transforms)...) {}

    template <class DST, class SRC>
    DST to(const SRC& p) const {
        using edges = typename compose_detail::edges_of_all<
            std::index_sequence_for<TRANSFORMS...>, TRANSFORMS...>::type;
        using path = typename compose_detail::find_path<SRC, DST, edges,
                                                        compose_detail::type_list<SRC>>::type;
        static_assert(!std::is_same_v<path, compose_detail::no_path>,
                      "no composition of the given transforms leads from SRC to DST");
        return compose_detail::path_runner<path>::run(transforms_, p);
    }

    template <class DST, class SRC>
    DST operator()(const SRC& p, Tag<DST>) const {
        return to<DST>(p);
    }

private:
    std::tuple<TRANSFORMS...> transforms_;
};

template <class... TRANSFORMS>
Composed<std::decay_t<TRANSFORMS>...> compose_transforms(TRANSFORMS&&... transforms) {
    return Composed<std::decay_t<TRANSFORMS>...>(std::forward<TRANSFORMS>(transforms)...);
}

// Attaches explicit metadata to a callable whose signature cannot be
// inferred, e.g. a generic lambda.
template <class DST, class SRC, class F>
class TransformAdapter {
public:
    using transformations = transform_list<transform_spec<DST, SRC>>;

    explicit TransformAdapter(F f) : f_(std::move(f)) {}

    DST operator()(const SRC& p, Tag<DST> = {}) const {
        return f_(p);
    }

private:
    F f_;
};

template <class DST, class SRC, class F>
TransformAdapter<DST, SRC, std::decay_t<F>> make_transform(F&& f) {
    return TransformAdapter<DST, SRC, std::decay_t<F>>(std::forward<F>(f));
}
