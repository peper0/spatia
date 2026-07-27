#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

// The actual definition may appear after this header.
template <class T>
struct Tag;

template <class Destination, class Source>
struct transform_spec {
    using destination_type = Destination;
    using source_type = Source;
};

template <class... Specs>
struct transform_list {};

// Selects one overload from a free-function overload set.
// Example:
//   overload_cast<A(const C&, Tag<A>)>(convert)
template <class Signature>
struct overload_cast_t;

template <class R, class... Args>
struct overload_cast_t<R(Args...)> {
    constexpr auto operator()(R (*function)(Args...)) const noexcept
        -> R (*)(Args...)
    {
        return function;
    }
};

template <class R, class... Args>
struct overload_cast_t<R(Args...) noexcept> {
    constexpr auto operator()(R (*function)(Args...) noexcept) const noexcept
        -> R (*)(Args...) noexcept
    {
        return function;
    }
};

template <class Signature>
inline constexpr overload_cast_t<Signature> overload_cast{};

namespace compose_detail {

template <class... Ts>
struct type_list {};

template <class T, class List>
struct contains;

template <class T, class... Ts>
struct contains<T, type_list<Ts...>>
    : std::bool_constant<(std::same_as<T, Ts> || ...)> {};

template <class T, class List>
inline constexpr bool contains_v = contains<T, List>::value;

template <class T, class List>
struct push_front;

template <class T, class... Ts>
struct push_front<T, type_list<Ts...>> {
    using type = type_list<T, Ts...>;
};

template <class T, class List>
using push_front_t = typename push_front<T, List>::type;

template <class...>
inline constexpr bool always_false_v = false;

// Extract SRC and DST from one non-overloaded callable signature.
// Supported forms:
//   DST callable(const SRC&)
//   DST callable(const SRC&, Tag<DST>)
template <class>
struct callable_traits;

template <class R, class FirstArg>
struct callable_traits<R(FirstArg)> {
    using source_type = std::remove_cvref_t<FirstArg>;
    using destination_type = std::remove_cvref_t<R>;
};

template <class R, class FirstArg, class SecondArg>
struct callable_traits<R(FirstArg, SecondArg)> {
    using source_type = std::remove_cvref_t<FirstArg>;
    using destination_type = std::remove_cvref_t<R>;

    static_assert(
        std::same_as<std::remove_cvref_t<SecondArg>, Tag<destination_type>>,
        "The second transform argument must be Tag<DST>");
};

template <class R, class FirstArg, class SecondArg, class... Rest>
struct callable_traits<R(FirstArg, SecondArg, Rest...)> {
    static_assert(
        always_false_v<R, FirstArg, SecondArg, Rest...>,
        "A transform callable must accept SRC or SRC, Tag<DST>");
};

template <class R, class... Args>
struct callable_traits<R (*)(Args...)> : callable_traits<R(Args...)> {};

template <class R, class... Args>
struct callable_traits<R (*)(Args...) noexcept>
    : callable_traits<R(Args...)> {};

template <class R, class... Args>
struct callable_traits<R (&)(Args...)> : callable_traits<R(Args...)> {};

template <class R, class... Args>
struct callable_traits<R (&)(Args...) noexcept>
    : callable_traits<R(Args...)> {};

template <class R, class Class, class... Args>
struct callable_traits<R (Class::*)(Args...)> : callable_traits<R(Args...)> {};

template <class R, class Class, class... Args>
struct callable_traits<R (Class::*)(Args...) const>
    : callable_traits<R(Args...)> {};

template <class R, class Class, class... Args>
struct callable_traits<R (Class::*)(Args...) noexcept>
    : callable_traits<R(Args...)> {};

template <class R, class Class, class... Args>
struct callable_traits<R (Class::*)(Args...) const noexcept>
    : callable_traits<R(Args...)> {};

template <class R, class Class, class... Args>
struct callable_traits<R (Class::*)(Args...) &>
    : callable_traits<R(Args...)> {};

template <class R, class Class, class... Args>
struct callable_traits<R (Class::*)(Args...) const &>
    : callable_traits<R(Args...)> {};

template <class R, class Class, class... Args>
struct callable_traits<R (Class::*)(Args...) & noexcept>
    : callable_traits<R(Args...)> {};

template <class R, class Class, class... Args>
struct callable_traits<R (Class::*)(Args...) const & noexcept>
    : callable_traits<R(Args...)> {};

template <class T, class = void>
struct inferred_callable_traits {
    static_assert(
        always_false_v<T>,
        "Cannot infer a transform edge. Use a non-overloaded function/lambda, "
        "declare `using transformations = transform_list<...>`, or wrap the "
        "callable with make_transform<DST, SRC>().");
};

template <class T>
struct inferred_callable_traits<
    T,
    std::enable_if_t<
        std::is_pointer_v<T> &&
        std::is_function_v<std::remove_pointer_t<T>>>>
    : callable_traits<T> {};

template <class T>
struct inferred_callable_traits<T, std::void_t<decltype(&T::operator())>>
    : callable_traits<decltype(&T::operator())> {};

template <class T>
struct normalize_transform_list {
    static_assert(
        always_false_v<T>,
        "transformations must be transform_list<transform_spec<...>, ...>");
};

template <class... Specs>
struct normalize_transform_list<transform_list<Specs...>> {
    static_assert(
        (requires {
            typename Specs::source_type;
            typename Specs::destination_type;
        } && ...),
        "Every item in transform_list must provide source_type and destination_type");

    using type = transform_list<Specs...>;
};

template <class Transform>
concept has_declared_transformations = requires {
    typename std::remove_cvref_t<Transform>::transformations;
};

template <class Transform, bool = has_declared_transformations<Transform>>
struct transform_specs;

template <class Transform>
struct transform_specs<Transform, true> {
private:
    using declared = typename std::remove_cvref_t<Transform>::transformations;

public:
    using type = typename normalize_transform_list<declared>::type;
};

template <class Transform>
struct transform_specs<Transform, false> {
private:
    using traits = inferred_callable_traits<std::remove_cvref_t<Transform>>;

public:
    using type = transform_list<transform_spec<
        typename traits::destination_type,
        typename traits::source_type>>;
};

template <std::size_t OwnerIndex, class Destination, class Source>
struct edge {
    static constexpr std::size_t owner_index = OwnerIndex;
    using source_type = Source;
    using destination_type = Destination;
};

template <class... Edges>
struct edge_list {};

struct no_path {};

template <class Left, class Right>
struct concat_edges;

template <class... Left, class... Right>
struct concat_edges<edge_list<Left...>, edge_list<Right...>> {
    using type = edge_list<Left..., Right...>;
};

template <class Left, class Right>
using concat_edges_t = typename concat_edges<Left, Right>::type;

template <class Edge, class List>
struct prepend_edge;

template <class Edge, class... Edges>
struct prepend_edge<Edge, edge_list<Edges...>> {
    using type = edge_list<Edge, Edges...>;
};

template <class Edge, class List>
using prepend_edge_t = typename prepend_edge<Edge, List>::type;

template <std::size_t OwnerIndex, class Specs>
struct bind_edges;

template <std::size_t OwnerIndex, class... Specs>
struct bind_edges<OwnerIndex, transform_list<Specs...>> {
    using type = edge_list<edge<
        OwnerIndex,
        typename Specs::destination_type,
        typename Specs::source_type>...>;
};

template <std::size_t OwnerIndex, class... Transforms>
struct collect_edges;

template <std::size_t OwnerIndex>
struct collect_edges<OwnerIndex> {
    using type = edge_list<>;
};

template <std::size_t OwnerIndex, class Transform, class... Rest>
struct collect_edges<OwnerIndex, Transform, Rest...> {
private:
    using current = typename bind_edges<
        OwnerIndex,
        typename transform_specs<Transform>::type>::type;
    using remaining = typename collect_edges<OwnerIndex + 1, Rest...>::type;

public:
    using type = concat_edges_t<current, remaining>;
};

template <class... Transforms>
using collect_edges_t = typename collect_edges<0, Transforms...>::type;

// Adjacency lookup is cached per (graph, source type). The route search no
// longer scans unrelated edges at every recursive step.
template <class Source, class Edges>
struct outgoing_edges;

template <class Source>
struct outgoing_edges<Source, edge_list<>> {
    using type = edge_list<>;
};

template <class Source, class Head, class... Tail>
struct outgoing_edges<Source, edge_list<Head, Tail...>> {
private:
    using remaining = typename outgoing_edges<Source, edge_list<Tail...>>::type;

public:
    using type = std::conditional_t<
        std::same_as<Source, typename Head::source_type>,
        prepend_edge_t<Head, remaining>,
        remaining>;
};

template <class Source, class Edges>
using outgoing_edges_t = typename outgoing_edges<Source, Edges>::type;

template <class Route, class Visited>
struct search_result {
    using route = Route;
    using visited = Visited;
};

template <class Route>
inline constexpr bool route_found_v = !std::same_as<Route, no_path>;

template <class Graph, class Current, class Destination, class Visited>
struct find_route;

template <
    class Graph,
    class Candidates,
    class Destination,
    class Visited>
struct search_candidates;

template <class Graph, class Destination, class Visited>
struct search_candidates<Graph, edge_list<>, Destination, Visited> {
    using type = search_result<no_path, Visited>;
};

template <
    bool ChildFound,
    class Graph,
    class Head,
    class Tail,
    class Destination,
    class ChildResult>
struct select_candidate;

template <
    class Graph,
    class Head,
    class Tail,
    class Destination,
    class ChildResult>
struct select_candidate<
    true,
    Graph,
    Head,
    Tail,
    Destination,
    ChildResult> {
    using type = search_result<
        prepend_edge_t<Head, typename ChildResult::route>,
        typename ChildResult::visited>;
};

template <
    class Graph,
    class Head,
    class Tail,
    class Destination,
    class ChildResult>
struct select_candidate<
    false,
    Graph,
    Head,
    Tail,
    Destination,
    ChildResult> {
    using type = typename search_candidates<
        Graph,
        Tail,
        Destination,
        typename ChildResult::visited>::type;
};

template <
    class Graph,
    class Head,
    class... Tail,
    class Destination,
    class Visited>
struct search_candidates<
    Graph,
    edge_list<Head, Tail...>,
    Destination,
    Visited> {
private:
    using child_result = typename find_route<
        Graph,
        typename Head::destination_type,
        Destination,
        Visited>::type;

public:
    using type = typename select_candidate<
        route_found_v<typename child_result::route>,
        Graph,
        Head,
        edge_list<Tail...>,
        Destination,
        child_result>::type;
};

template <
    class Graph,
    class Current,
    class Destination,
    class Visited,
    bool AtDestination = std::same_as<Current, Destination>,
    bool AlreadyVisited = contains_v<Current, Visited>>
struct find_route_impl;

template <
    class Graph,
    class Current,
    class Destination,
    class Visited,
    bool AlreadyVisited>
struct find_route_impl<
    Graph,
    Current,
    Destination,
    Visited,
    true,
    AlreadyVisited> {
    using type = search_result<edge_list<>, Visited>;
};

template <
    class Graph,
    class Current,
    class Destination,
    class Visited>
struct find_route_impl<
    Graph,
    Current,
    Destination,
    Visited,
    false,
    true> {
    using type = search_result<no_path, Visited>;
};

template <
    class Graph,
    class Current,
    class Destination,
    class Visited>
struct find_route_impl<
    Graph,
    Current,
    Destination,
    Visited,
    false,
    false> {
private:
    using next_visited = push_front_t<Current, Visited>;
    using candidates = outgoing_edges_t<Current, Graph>;

public:
    using type = typename search_candidates<
        Graph,
        candidates,
        Destination,
        next_visited>::type;
};

template <class Graph, class Current, class Destination, class Visited>
struct find_route
    : find_route_impl<Graph, Current, Destination, Visited> {};

template <class Graph, class Source, class Destination>
using route_t = typename find_route<
    Graph,
    Source,
    Destination,
    type_list<>>::type::route;

template <class Graph, class Source, class Destination>
inline constexpr bool has_route_v = route_found_v<
    route_t<Graph, Source, Destination>>;

// Prefer the tagged form because it disambiguates an overloaded operator().
template <class Destination, class Callable, class Source>
constexpr Destination invoke_transform(
    const Callable& callable,
    const Source& source)
{
    if constexpr (requires {
                      {
                          std::invoke(callable, source, Tag<Destination>{})
                      } -> std::same_as<Destination>;
                  }) {
        return std::invoke(callable, source, Tag<Destination>{});
    } else {
        static_assert(
            requires {
                { std::invoke(callable, source) } -> std::same_as<Destination>;
            },
            "A transform_spec does not match the callable. Expected either "
            "DST(const SRC&) or DST(const SRC&, Tag<DST>).");

        return std::invoke(callable, source);
    }
}

} // namespace compose_detail

// Adds explicit SRC/DST metadata to a generic lambda or another callable whose
// signature cannot be inferred automatically.
template <class Destination, class Source, class Callable>
class ExplicitTransform {
public:
    using transformations = transform_list<transform_spec<Destination, Source>>;

    explicit constexpr ExplicitTransform(Callable callable)
        : callable_(std::move(callable))
    {}

    constexpr Destination operator()(
        const Source& source,
        Tag<Destination> = {}) const
    {
        return compose_detail::invoke_transform<Destination>(callable_, source);
    }

private:
    Callable callable_;
};

template <class Destination, class Source, class Callable>
constexpr auto make_transform(Callable&& callable)
{
    return ExplicitTransform<
        Destination,
        Source,
        std::decay_t<Callable>>{std::forward<Callable>(callable)};
}

template <class... Transforms>
class ComposedTransforms {
    using transforms_tuple = std::tuple<Transforms...>;
    using edges = compose_detail::collect_edges_t<Transforms...>;

    template <class Source, class Destination>
    using route_for = compose_detail::route_t<edges, Source, Destination>;

    template <class Source, class Destination>
    static constexpr bool can_transform =
        compose_detail::route_found_v<route_for<Source, Destination>>;

public:
    template <class... Ts>
        requires (sizeof...(Ts) == sizeof...(Transforms))
    explicit constexpr ComposedTransforms(Ts&&... transforms)
        : transforms_(std::forward<Ts>(transforms)...)
    {}

    template <class Destination, class Source>
        requires can_transform<std::remove_cvref_t<Source>, Destination>
    constexpr Destination operator()(
        const Source& source,
        Tag<Destination> = {}) const
    {
        using source_type = std::remove_cvref_t<Source>;
        using route = route_for<source_type, Destination>;

        static_assert(compose_detail::route_found_v<route>);
        return execute_route(source, route{});
    }

    template <class Destination, class Source>
        requires can_transform<std::remove_cvref_t<Source>, Destination>
    constexpr Destination transform(
        const Source& source,
        Tag<Destination> tag = {}) const
    {
        return (*this)(source, tag);
    }

    template <class Destination, class Source>
        requires can_transform<std::remove_cvref_t<Source>, Destination>
    constexpr Destination to(const Source& source) const
    {
        return (*this)(source, Tag<Destination>{});
    }

private:
    template <class Current>
    constexpr Current execute_route(
        const Current& current,
        compose_detail::edge_list<>) const
    {
        return current;
    }

    template <class Current, class Head, class... Tail>
    constexpr auto execute_route(
        const Current& current,
        compose_detail::edge_list<Head, Tail...>) const
    {
        static_assert(
            std::same_as<Current, typename Head::source_type>,
            "Internal compose route mismatch");

        const auto& transformer = std::get<Head::owner_index>(transforms_);
        const auto next = compose_detail::invoke_transform<
            typename Head::destination_type>(transformer, current);

        return execute_route(
            next,
            compose_detail::edge_list<Tail...>{});
    }

    transforms_tuple transforms_;
};

template <class... Transforms>
constexpr auto compose_transforms(Transforms&&... transforms)
{
    return ComposedTransforms<std::decay_t<Transforms>...>{
        std::forward<Transforms>(transforms)...};
}