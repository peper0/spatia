#pragma once

#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "spatia/transforms/detail/transform_graph_fwd.hpp"
#include "spatia/transforms/detail/transform_traits.hpp"

namespace spatia {

/// Selects one overload of an ordinary function by its exact signature.
template <class Signature>
constexpr Signature* overload_cast(Signature* fn) noexcept {
    return fn;
}

namespace detail {

// A conversion edge bound to the transform stored at Index in the tuple.
template <std::size_t Index, class Source, class Destination>
struct Edge {
    static constexpr std::size_t index = Index;
    using Src = Source;
    using Dst = Destination;
};

template <class... Edges>
struct EdgeList {};

template <class... Lists>
struct Concat;

template <>
struct Concat<> {
    using Type = EdgeList<>;
};

template <class List>
struct Concat<List> {
    using Type = List;
};

template <class... As, class... Bs, class... Rest>
struct Concat<EdgeList<As...>, EdgeList<Bs...>, Rest...> : Concat<EdgeList<As..., Bs...>, Rest...> {};

template <std::size_t Index, class SpecList>
struct EdgesOfOne;

template <std::size_t Index, class... Specs>
struct EdgesOfOne<Index, TransformList<Specs...>> {
    using Type = EdgeList<Edge<Index, typename Specs::Src, typename Specs::Dst>...>;
};

template <class Seq, class... Ts>
struct EdgesOfAll;

template <std::size_t... Is, class... Ts>
struct EdgesOfAll<std::index_sequence<Is...>, Ts...> {
    using Type = typename Concat<typename EdgesOfOne<Is, typename SpecsOf<Ts>::Type>::Type...>::Type;
};

// The list of distinct types appearing in the edges; a type's position in it
// is its node id in the value-level graph below.
template <class... Ts>
struct TypeList {};

template <class List>
struct ListSize;

template <class... Ts>
struct ListSize<TypeList<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <class List, class T>
struct AddUnique;

template <class... Ts, class T>
struct AddUnique<TypeList<Ts...>, T> {
    using Type = std::conditional_t<(std::is_same_v<T, Ts> || ... || false), TypeList<Ts...>, TypeList<Ts..., T>>;
};

template <class List, class Edges>
struct CollectNodes;

template <class List>
struct CollectNodes<List, EdgeList<>> {
    using Type = List;
};

template <class List, class E, class... Rest>
struct CollectNodes<List, EdgeList<E, Rest...>>
    : CollectNodes<typename AddUnique<typename AddUnique<List, typename E::Src>::Type, typename E::Dst>::Type,
                   EdgeList<Rest...>> {};

template <class Edges>
struct NodesOf : CollectNodes<TypeList<>, Edges> {};

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
template <class T, class List>
struct IndexOf;

template <class T, class... Ts>
struct IndexOf<T, TypeList<Ts...>> {
    static constexpr std::size_t value = first_true(std::array<bool, sizeof...(Ts)>{std::is_same_v<T, Ts>...});
};

// The graph as plain values. All path searching happens on this table inside
// constexpr evaluation, which is far cheaper for the compiler than a search
// spelled with template instantiations: no visited-set types, no
// per-candidate-edge instantiations, and intermediate search states leave
// nothing behind in the instantiation cache.
struct EdgeDesc {
    std::size_t src;
    std::size_t dst;
};

template <class Nodes, class Edges>
struct EdgeTable;

template <class Nodes, class... Es>
struct EdgeTable<Nodes, EdgeList<Es...>> {
    static constexpr std::array<EdgeDesc, sizeof...(Es)> edges = {
        EdgeDesc{IndexOf<typename Es::Src, Nodes>::value, IndexOf<typename Es::Dst, Nodes>::value}...};
};

// A simple path visits every node at most once, so NodeCount bounds its
// length. Structural type, so a constexpr result can be a template argument.
template <std::size_t NodeCount>
struct PathResult {
    std::array<std::size_t, NodeCount> edges{};
    std::size_t length = 0;
    bool found = false;
};

// Breadth-first search from node `from` to node `to`; returns the edge
// indices of a shortest path.
template <std::size_t NodeCount, std::size_t EdgeCount>
constexpr PathResult<NodeCount> find_path(const std::array<EdgeDesc, EdgeCount>& edges, std::size_t from,
                                          std::size_t to) {
    PathResult<NodeCount> result{};
    if (from >= NodeCount || to >= NodeCount || from == to) {
        result.found = from == to && from < NodeCount;
        return result;
    }
    std::array<std::size_t, NodeCount> parent_edge{};
    std::array<bool, NodeCount> reached{};
    std::array<std::size_t, NodeCount> queue{};
    std::size_t head = 0;
    std::size_t tail = 0;
    reached[from] = true;
    queue[tail++] = from;
    while (head < tail && !result.found) {
        const std::size_t node = queue[head++];
        for (std::size_t e = 0; e < EdgeCount; ++e) {
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
template <auto Path, std::size_t... Is>
constexpr auto path_edge_seq(std::index_sequence<Is...>) -> std::index_sequence<Path.edges[Is]...>;

// Calls one transform, preferring the tagged form so that overloaded
// callables resolve to the requested destination.
template <class Destination, class T, class Source>
Destination apply_one(const T& t, const Source& p) {
    if constexpr (std::is_invocable_v<const T&, const Source&, Tag<Destination>>) {
        return t(p, Tag<Destination>{});
    } else {
        return t(p);
    }
}

template <class Edges>
struct PathRunner;

template <class... Es>
struct PathRunner<EdgeList<Es...>> {
    using EdgeTuple = std::tuple<Es...>;

    template <class Tuple, class P>
    static const P& run(std::index_sequence<>, const Tuple&, const P& p) {
        return p;
    }

    template <std::size_t First, std::size_t... Rest, class Tuple, class P>
    static auto run(std::index_sequence<First, Rest...>, const Tuple& transforms, const P& p) {
        using E = std::tuple_element_t<First, EdgeTuple>;
        return run(std::index_sequence<Rest...>{}, transforms,
                   apply_one<typename E::Dst>(std::get<E::index>(transforms), p));
    }
};

}  // namespace detail

/// The transform returned by `combine`: it provides every conversion of its
/// operands plus every conversion reachable by chaining them.
/// `to<Destination>()` finds a shortest path of declared edges from the
/// argument's type to `Destination` at compile time and applies the operands
/// along it.
template <class... Transforms>
class Composed {
   public:
    explicit Composed(Transforms... transforms) : transforms_(std::move(transforms)...) {}

    template <class Destination, class Source>
    Destination to(const Source& p) const {
        if constexpr (std::is_same_v<Destination, Source>) {
            return p;
        } else {
            static constexpr auto path = detail::find_path<node_count>(
                Table::edges, detail::IndexOf<Source, Nodes>::value, detail::IndexOf<Destination, Nodes>::value);
            static_assert(path.found,
                          "no composition of the given transforms leads from the "
                          "source type to the destination type");
            using Seq = decltype(detail::path_edge_seq<path>(std::make_index_sequence<path.length>{}));
            return detail::PathRunner<Edges>::run(Seq{}, transforms_, p);
        }
    }

    template <class Destination, class Source>
    Destination operator()(const Source& p, Tag<Destination>) const {
        return to<Destination>(p);
    }

   private:
    using Edges = typename detail::EdgesOfAll<std::index_sequence_for<Transforms...>, Transforms...>::Type;
    using Nodes = typename detail::NodesOf<Edges>::Type;
    static constexpr std::size_t node_count = detail::ListSize<Nodes>::value;
    using Table = detail::EdgeTable<Nodes, Edges>;

    std::tuple<Transforms...> transforms_;
};

/// Builds a transform providing every conversion of the given transforms plus
/// all of their chained compositions. Unlike `compose(ab, bc)`, which merges
/// exactly two transforms into a single one, `combine` accepts any number of
/// them and keeps each as a separate conversion edge.
template <class... Transforms>
Composed<std::decay_t<Transforms>...> combine(Transforms&&... transforms) {
    return Composed<std::decay_t<Transforms>...>(std::forward<Transforms>(transforms)...);
}

/// Attaches explicit metadata to a callable whose signature cannot be
/// inferred, e.g. a generic lambda.
template <class Destination, class Source, class F>
class TransformAdapter {
   public:
    using Transformations = TransformList<TransformSpec<Destination, Source>>;

    explicit TransformAdapter(F f) : f_(std::move(f)) {}

    Destination operator()(const Source& p, Tag<Destination> = {}) const { return f_(p); }

   private:
    F f_;
};

template <class Destination, class Source, class F>
TransformAdapter<Destination, Source, std::decay_t<F>> make_transform(F&& f) {
    return TransformAdapter<Destination, Source, std::decay_t<F>>(std::forward<F>(f));
}

}  // namespace spatia
