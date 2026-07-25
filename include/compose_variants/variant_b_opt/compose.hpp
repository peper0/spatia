#pragma once

#include <array>
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

// The list of distinct types appearing in the edges; a type's position in it
// is its node id in the value-level graph below.
template <class... TS>
struct type_list {};

template <class LIST>
struct list_size;

template <class... TS>
struct list_size<type_list<TS...>> : std::integral_constant<std::size_t, sizeof...(TS)> {};

template <class LIST, class T>
struct add_unique;

template <class... TS, class T>
struct add_unique<type_list<TS...>, T> {
    using type = std::conditional_t<(std::is_same_v<T, TS> || ... || false),
                                    type_list<TS...>, type_list<TS..., T>>;
};

template <class LIST, class EDGES>
struct collect_nodes;

template <class LIST>
struct collect_nodes<LIST, edge_list<>> {
    using type = LIST;
};

template <class LIST, class E, class... REST>
struct collect_nodes<LIST, edge_list<E, REST...>>
    : collect_nodes<typename add_unique<typename add_unique<LIST, typename E::src>::type,
                                        typename E::dst>::type,
                    edge_list<REST...>> {};

template <class EDGES>
struct nodes_of : collect_nodes<type_list<>, EDGES> {};

template <std::size_t N>
constexpr std::size_t first_true(const std::array<bool, N>& flags) {
    for (std::size_t i = 0; i < N; ++i) {
        if (flags[i]) {
            return i;
        }
    }
    return N;
}

// Node id of T, or the node count when T does not occur in the graph.
template <class T, class LIST>
struct index_of;

template <class T, class... TS>
struct index_of<T, type_list<TS...>> {
    static constexpr std::size_t value =
        first_true(std::array<bool, sizeof...(TS)>{std::is_same_v<T, TS>...});
};

// The graph as plain values. All path searching happens on this table inside
// constexpr evaluation, which is far cheaper for the compiler than a search
// spelled with template instantiations: no visited-set types, no
// per-candidate-edge instantiations, and intermediate search states leave
// nothing behind in the instantiation cache.
struct edge_desc {
    std::size_t src;
    std::size_t dst;
};

template <class NODES, class EDGES>
struct edge_table;

template <class NODES, class... ES>
struct edge_table<NODES, edge_list<ES...>> {
    static constexpr std::array<edge_desc, sizeof...(ES)> edges = {
        edge_desc{index_of<typename ES::src, NODES>::value,
                  index_of<typename ES::dst, NODES>::value}...};
};

// A simple path visits every node at most once, so NODE_COUNT bounds its
// length. Structural type, so a constexpr result can be a template argument.
template <std::size_t NODE_COUNT>
struct path_result {
    std::array<std::size_t, NODE_COUNT> edges{};
    std::size_t length = 0;
    bool found = false;
};

// Breadth-first search from node `from` to node `to`; returns the edge
// indices of a shortest path.
template <std::size_t NODE_COUNT, std::size_t EDGE_COUNT>
constexpr path_result<NODE_COUNT> find_path(const std::array<edge_desc, EDGE_COUNT>& edges,
                                            std::size_t from, std::size_t to) {
    path_result<NODE_COUNT> result{};
    if (from >= NODE_COUNT || to >= NODE_COUNT || from == to) {
        result.found = from == to && from < NODE_COUNT;
        return result;
    }
    std::array<std::size_t, NODE_COUNT> parent_edge{};
    std::array<bool, NODE_COUNT> reached{};
    std::array<std::size_t, NODE_COUNT> queue{};
    std::size_t head = 0;
    std::size_t tail = 0;
    reached[from] = true;
    queue[tail++] = from;
    while (head < tail && !result.found) {
        const std::size_t node = queue[head++];
        for (std::size_t e = 0; e < EDGE_COUNT; ++e) {
            if (edges[e].src != node || reached[edges[e].dst]) {
                continue;
            }
            reached[edges[e].dst] = true;
            parent_edge[edges[e].dst] = e;
            if (edges[e].dst == to) {
                result.found = true;
                break;
            }
            queue[tail++] = edges[e].dst;
        }
    }
    if (result.found) {
        std::size_t length = 0;
        for (std::size_t n = to; n != from; n = edges[parent_edge[n]].src) {
            ++length;
        }
        result.length = length;
        for (std::size_t n = to; n != from; n = edges[parent_edge[n]].src) {
            result.edges[--length] = parent_edge[n];
        }
    }
    return result;
}

// Lifts the found path back to compile time as a pack of edge indices.
template <auto PATH, std::size_t... IS>
constexpr auto path_edge_seq(std::index_sequence<IS...>)
    -> std::index_sequence<PATH.edges[IS]...>;

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

template <class EDGES>
struct path_runner;

template <class... ES>
struct path_runner<edge_list<ES...>> {
    using edge_tuple = std::tuple<ES...>;

    template <class TUPLE, class P>
    static const P& run(std::index_sequence<>, const TUPLE&, const P& p) {
        return p;
    }

    template <std::size_t FIRST, std::size_t... REST, class TUPLE, class P>
    static auto run(std::index_sequence<FIRST, REST...>, const TUPLE& transforms, const P& p) {
        using E = std::tuple_element_t<FIRST, edge_tuple>;
        return run(std::index_sequence<REST...>{}, transforms,
                   apply_one<typename E::dst>(std::get<E::index>(transforms), p));
    }
};

}  // namespace compose_detail

template <class... TRANSFORMS>
class Composed {
public:
    explicit Composed(TRANSFORMS... transforms) : transforms_(std::move(transforms)...) {}

    template <class DST, class SRC>
    DST to(const SRC& p) const {
        if constexpr (std::is_same_v<DST, SRC>) {
            return p;
        } else {
            static constexpr auto path = compose_detail::find_path<node_count>(
                table::edges, compose_detail::index_of<SRC, nodes>::value,
                compose_detail::index_of<DST, nodes>::value);
            static_assert(path.found,
                          "no composition of the given transforms leads from SRC to DST");
            using seq = decltype(compose_detail::path_edge_seq<path>(
                std::make_index_sequence<path.length>{}));
            return compose_detail::path_runner<edges>::run(seq{}, transforms_, p);
        }
    }

    template <class DST, class SRC>
    DST operator()(const SRC& p, Tag<DST>) const {
        return to<DST>(p);
    }

private:
    using edges = typename compose_detail::edges_of_all<std::index_sequence_for<TRANSFORMS...>,
                                                        TRANSFORMS...>::type;
    using nodes = typename compose_detail::nodes_of<edges>::type;
    static constexpr std::size_t node_count = compose_detail::list_size<nodes>::value;
    using table = compose_detail::edge_table<nodes, edges>;

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
