#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

// This header is included before the application's definition of Tag.
// A declaration is sufficient until a composed transform is invoked.
template <class T>
struct Tag;

// Template arguments follow the same order as make_transform<Dst, Src>().
template <class Dst, class Src>
struct transform_spec {
    using destination_type = Dst;
    using source_type = Src;
};

template <class... TransformSpecs>
struct transform_list {};

namespace compose_detail {

inline constexpr std::size_t no_index = static_cast<std::size_t>(-1);

template <class... Ts>
struct type_list {};

template <class Signature>
struct signature_transformations;

template <class Result, class FirstArg, class... Rest>
struct signature_transformations<Result(FirstArg, Rest...)> {
    using type = transform_list<
        transform_spec<std::remove_cvref_t<Result>,
                       std::remove_cvref_t<FirstArg>>>;
};

template <class Callable>
struct callable_signature;

template <class Result, class... Args>
struct callable_signature<Result(Args...)> {
    using type = Result(Args...);
};

template <class Result, class... Args>
struct callable_signature<Result (*)(Args...)>
    : callable_signature<Result(Args...)> {};

template <class Result, class... Args>
struct callable_signature<Result (*)(Args...) noexcept>
    : callable_signature<Result(Args...)> {};

template <class Result, class... Args>
struct callable_signature<Result (&)(Args...)>
    : callable_signature<Result(Args...)> {};

template <class Result, class... Args>
struct callable_signature<Result (&)(Args...) noexcept>
    : callable_signature<Result(Args...)> {};

#define COMPOSE_MEMBER_SIGNATURE(cv_ref, exception_spec)                       \
    template <class Class, class Result, class... Args>                        \
    struct callable_signature<                                                  \
        Result (Class::*)(Args...) cv_ref exception_spec>                      \
        : callable_signature<Result(Args...)> {};

COMPOSE_MEMBER_SIGNATURE(, )
COMPOSE_MEMBER_SIGNATURE(const, )
COMPOSE_MEMBER_SIGNATURE(volatile, )
COMPOSE_MEMBER_SIGNATURE(const volatile, )
COMPOSE_MEMBER_SIGNATURE(&, )
COMPOSE_MEMBER_SIGNATURE(const &, )
COMPOSE_MEMBER_SIGNATURE(volatile &, )
COMPOSE_MEMBER_SIGNATURE(const volatile &, )
COMPOSE_MEMBER_SIGNATURE(&&, )
COMPOSE_MEMBER_SIGNATURE(const &&, )
COMPOSE_MEMBER_SIGNATURE(volatile &&, )
COMPOSE_MEMBER_SIGNATURE(const volatile &&, )
COMPOSE_MEMBER_SIGNATURE(, noexcept)
COMPOSE_MEMBER_SIGNATURE(const, noexcept)
COMPOSE_MEMBER_SIGNATURE(volatile, noexcept)
COMPOSE_MEMBER_SIGNATURE(const volatile, noexcept)
COMPOSE_MEMBER_SIGNATURE(&, noexcept)
COMPOSE_MEMBER_SIGNATURE(const &, noexcept)
COMPOSE_MEMBER_SIGNATURE(volatile &, noexcept)
COMPOSE_MEMBER_SIGNATURE(const volatile &, noexcept)
COMPOSE_MEMBER_SIGNATURE(&&, noexcept)
COMPOSE_MEMBER_SIGNATURE(const &&, noexcept)
COMPOSE_MEMBER_SIGNATURE(volatile &&, noexcept)
COMPOSE_MEMBER_SIGNATURE(const volatile &&, noexcept)

#undef COMPOSE_MEMBER_SIGNATURE

template <class Callable>
struct callable_signature
    : callable_signature<decltype(&std::remove_cvref_t<Callable>::operator())> {
};

template <class Callable, class = void>
struct transformations_of {
private:
    using signature = typename callable_signature<
        std::remove_cvref_t<Callable>>::type;

public:
    using type = typename signature_transformations<signature>::type;
};

template <class Callable>
struct transformations_of<
    Callable,
    std::void_t<typename std::remove_cvref_t<Callable>::transformations>> {
    using type = typename std::remove_cvref_t<Callable>::transformations;
};

template <class Callable>
using transformations_of_t = typename transformations_of<Callable>::type;

// Keep the owner index on the edge. Applying a path can then address the
// callable tuple directly instead of scanning every callable at every step.
template <class Spec, std::size_t CallableIndex>
struct bound_edge {
    using destination_type = typename Spec::destination_type;
    using source_type = typename Spec::source_type;
    static constexpr std::size_t callable_index = CallableIndex;
};

template <std::size_t CallableIndex, class TransformList>
struct bind_edges;

template <std::size_t CallableIndex, class... Specs>
struct bind_edges<CallableIndex, transform_list<Specs...>> {
    using type = type_list<bound_edge<Specs, CallableIndex>...>;
};

template <class... Lists>
struct concatenate_type_lists;

template <>
struct concatenate_type_lists<> {
    using type = type_list<>;
};

template <class... Head, class... Remaining>
struct concatenate_type_lists<type_list<Head...>, Remaining...> {
private:
    using tail = typename concatenate_type_lists<Remaining...>::type;

    template <class>
    struct prepend;

    template <class... Tail>
    struct prepend<type_list<Tail...>> {
        using type = type_list<Head..., Tail...>;
    };

public:
    using type = typename prepend<tail>::type;
};

template <class... Lists>
using concatenate_type_lists_t =
    typename concatenate_type_lists<Lists...>::type;

template <class IndexSequence, class... Callables>
struct make_edge_list;

template <std::size_t... Indices, class... Callables>
struct make_edge_list<std::index_sequence<Indices...>, Callables...> {
    using type = concatenate_type_lists_t<
        typename bind_edges<Indices, transformations_of_t<Callables>>::type...>;
};

template <class Needle, class... Haystack>
consteval std::size_t type_index(type_list<Haystack...>) {
    std::size_t result = no_index;
    std::size_t current = 0;
    ((result = result == no_index && std::is_same_v<Needle, Haystack>
                   ? current
                   : result,
      ++current),
     ...);
    return result;
}

struct edge_descriptor {
    std::size_t source;
    std::size_t destination;
};

struct next_edge_result {
    std::size_t edge_index{no_index};
    bool found{};
};

template <class EdgeList>
struct graph_data;

// Types are mapped to small integer ids once. Graph traversal below operates
// on constexpr arrays, so it does not recursively instantiate one template per
// visited edge.
template <class... Edges>
struct graph_data<type_list<Edges...>> {
    static constexpr std::size_t edge_count = sizeof...(Edges);
    static constexpr std::size_t node_capacity = 2 * edge_count;

    using edge_types = std::tuple<Edges...>;
    using endpoint_types = type_list<typename Edges::source_type...,
                                     typename Edges::destination_type...>;

    template <std::size_t Index>
    using edge_at = std::tuple_element_t<Index, edge_types>;

    inline static constexpr std::array<edge_descriptor, edge_count>
        descriptors{{
            edge_descriptor{
                type_index<typename Edges::source_type>(endpoint_types{}),
                type_index<typename Edges::destination_type>(endpoint_types{})}...}};

    struct adjacency_data {
        std::array<std::size_t, node_capacity + 1> offsets{};
        std::array<std::size_t, edge_count> edge_indices{};
    };

    static consteval adjacency_data build_incoming_adjacency() {
        adjacency_data result{};

        for (const auto& edge : descriptors) {
            ++result.offsets[edge.destination + 1];
        }
        for (std::size_t node = 1; node <= node_capacity; ++node) {
            result.offsets[node] += result.offsets[node - 1];
        }

        auto cursors = result.offsets;
        for (std::size_t edge = 0; edge < edge_count; ++edge) {
            const auto destination = descriptors[edge].destination;
            result.edge_indices[cursors[destination]++] = edge;
        }
        return result;
    }

    inline static constexpr adjacency_data incoming_adjacency =
        build_incoming_adjacency();

    // A reverse BFS is cached once per destination. All source types targeting
    // that destination reuse the same next-edge table; disconnected groups are
    // never traversed beyond their own component.
    template <class Destination>
    static consteval std::array<std::size_t, node_capacity>
    build_routes_to() {
        std::array<std::size_t, node_capacity> next_edges{};
        next_edges.fill(no_index);
        constexpr auto destination = type_index<Destination>(endpoint_types{});
        if (destination == no_index) {
            return next_edges;
        }

        std::array<bool, node_capacity> visited{};
        std::array<std::size_t, node_capacity> queue{};
        std::size_t queue_begin = 0;
        std::size_t queue_end = 0;
        visited[destination] = true;
        queue[queue_end++] = destination;

        while (queue_begin != queue_end) {
            const auto current = queue[queue_begin++];
            for (auto position = incoming_adjacency.offsets[current];
                 position < incoming_adjacency.offsets[current + 1];
                 ++position) {
                const auto edge_index =
                    incoming_adjacency.edge_indices[position];
                const auto previous = descriptors[edge_index].source;
                if (visited[previous]) {
                    continue;
                }
                visited[previous] = true;
                next_edges[previous] = edge_index;
                queue[queue_end++] = previous;
            }
        }
        return next_edges;
    }

    template <class Destination>
    inline static constexpr auto routes_to = build_routes_to<Destination>();

    template <class Source, class Destination>
    static consteval next_edge_result find_next_edge() {
        next_edge_result result{};
        if constexpr (std::is_same_v<Source, Destination>) {
            result.found = true;
            return result;
        }

        constexpr auto source = type_index<Source>(endpoint_types{});
        constexpr auto destination = type_index<Destination>(endpoint_types{});
        if (source == no_index || destination == no_index) {
            return result;
        }

        result.edge_index = routes_to<Destination>[source];
        if (result.edge_index != no_index) {
            result.found = true;
        }
        return result;
    }
};

template <class Dst, class Src, class Callable>
class explicit_transform {
public:
    using transformations = transform_list<transform_spec<Dst, Src>>;

    constexpr explicit_transform(Callable callable)
        : callable_(std::move(callable)) {}

    constexpr decltype(auto) operator()(const Src& source) {
        return std::invoke(callable_, source);
    }

    constexpr decltype(auto) operator()(const Src& source) const
        requires std::is_invocable_v<const Callable&, const Src&>
    {
        return std::invoke(callable_, source);
    }

private:
    [[no_unique_address]] Callable callable_;
};

template <class... Callables>
class composed_transform {
private:
    using all_edges = typename make_edge_list<
        std::index_sequence_for<Callables...>,
        Callables...>::type;
    using graph = graph_data<all_edges>;

    template <class Source, class Destination>
    inline static constexpr auto next_edge =
        graph::template find_next_edge<Source, Destination>();

public:
    constexpr explicit composed_transform(Callables... callables)
        : callables_(std::move(callables)...) {}

    template <class Dst, class Src>
    constexpr auto to(Src&& source) {
        using source_type = std::remove_cvref_t<Src>;
        static_assert(next_edge<source_type, Dst>.found,
                      "No transformation path exists between these types");
        return apply_route<source_type, Dst>(std::forward<Src>(source));
    }

    template <class Dst, class Src>
    constexpr auto to(Src&& source) const {
        using source_type = std::remove_cvref_t<Src>;
        static_assert(next_edge<source_type, Dst>.found,
                      "No transformation path exists between these types");
        return apply_route<source_type, Dst>(std::forward<Src>(source));
    }

    template <class Src, class Dst>
    constexpr auto operator()(Src&& source, Tag<Dst>) {
        return to<Dst>(std::forward<Src>(source));
    }

    template <class Src, class Dst>
    constexpr auto operator()(Src&& source, Tag<Dst>) const {
        return to<Dst>(std::forward<Src>(source));
    }

private:
    template <class Edge, class Value>
    constexpr decltype(auto) apply_edge(Value&& value) {
        auto&& callable = std::get<Edge::callable_index>(callables_);
        using destination = typename Edge::destination_type;

        if constexpr (std::is_invocable_v<
                          decltype(callable),
                          Value,
                          Tag<destination>>) {
            return std::invoke(callable,
                               std::forward<Value>(value),
                               Tag<destination>{});
        } else {
            return std::invoke(callable, std::forward<Value>(value));
        }
    }

    template <class Edge, class Value>
    constexpr decltype(auto) apply_edge(Value&& value) const {
        auto&& callable = std::get<Edge::callable_index>(callables_);
        using destination = typename Edge::destination_type;

        if constexpr (std::is_invocable_v<
                          decltype(callable),
                          Value,
                          Tag<destination>>) {
            return std::invoke(callable,
                               std::forward<Value>(value),
                               Tag<destination>{});
        } else {
            return std::invoke(callable, std::forward<Value>(value));
        }
    }

    template <class Source,
              class Destination,
              class Value>
    constexpr auto apply_route(Value&& value) {
        if constexpr (std::is_same_v<Source, Destination>) {
            return std::remove_cvref_t<Value>(std::forward<Value>(value));
        } else {
            static_assert(next_edge<Source, Destination>.found);
            constexpr auto edge_index =
                next_edge<Source, Destination>.edge_index;
            using edge = typename graph::template edge_at<edge_index>;
            return apply_route<typename edge::destination_type, Destination>(
                apply_edge<edge>(std::forward<Value>(value)));
        }
    }

    template <class Source,
              class Destination,
              class Value>
    constexpr auto apply_route(Value&& value) const {
        if constexpr (std::is_same_v<Source, Destination>) {
            return std::remove_cvref_t<Value>(std::forward<Value>(value));
        } else {
            static_assert(next_edge<Source, Destination>.found);
            constexpr auto edge_index =
                next_edge<Source, Destination>.edge_index;
            using edge = typename graph::template edge_at<edge_index>;
            return apply_route<typename edge::destination_type, Destination>(
                apply_edge<edge>(std::forward<Value>(value)));
        }
    }

    std::tuple<Callables...> callables_;
};

} // namespace compose_detail

template <class Signature>
constexpr auto overload_cast(Signature* function) noexcept -> Signature* {
    return function;
}

template <class Dst, class Src, class Callable>
constexpr auto make_transform(Callable&& callable) {
    using stored_callable = std::decay_t<Callable>;
    return compose_detail::explicit_transform<Dst, Src, stored_callable>(
        std::forward<Callable>(callable));
}

template <class... Callables>
constexpr auto compose_transforms(Callables&&... callables) {
    return compose_detail::composed_transform<std::decay_t<Callables>...>(
        std::forward<Callables>(callables)...);
}
