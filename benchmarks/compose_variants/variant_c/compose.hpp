#pragma once

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

struct no_path {};

template <class... Ts>
struct type_list {};

template <class T, class List>
struct type_list_contains;

template <class T, class... Ts>
struct type_list_contains<T, type_list<Ts...>>
    : std::bool_constant<(std::is_same_v<T, Ts> || ...)> {};

template <class T, class List>
inline constexpr bool type_list_contains_v = type_list_contains<T, List>::value;

template <class T, class List>
struct type_list_prepend;

template <class T, class... Ts>
struct type_list_prepend<T, type_list<Ts...>> {
    using type = type_list<T, Ts...>;
};

template <class T, class List>
using type_list_prepend_t = typename type_list_prepend<T, List>::type;

template <class List, class T>
struct type_list_append;

template <class... Ts, class T>
struct type_list_append<type_list<Ts...>, T> {
    using type = type_list<Ts..., T>;
};

template <class List, class T>
using type_list_append_t = typename type_list_append<List, T>::type;

template <class Spec, class TransformList>
struct transform_list_contains;

template <class Spec, class... Specs>
struct transform_list_contains<Spec, transform_list<Specs...>>
    : std::bool_constant<(std::is_same_v<Spec, Specs> || ...)> {};

template <class Spec, class TransformList>
inline constexpr bool transform_list_contains_v =
    transform_list_contains<Spec, TransformList>::value;

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

template <class Current, class Target, class AllEdges, class Visited>
struct find_path;

template <class Edge, class Tail, bool HasPath>
struct prepend_edge_to_path;

template <class Edge, class Tail>
struct prepend_edge_to_path<Edge, Tail, false> {
    using type = no_path;
};

template <class Edge, class Tail>
struct prepend_edge_to_path<Edge, Tail, true> {
    using type = type_list_prepend_t<Edge, Tail>;
};

template <bool Eligible,
          class Edge,
          class Target,
          class AllEdges,
          class Visited>
struct try_edge {
    using type = no_path;
};

template <class Edge, class Target, class AllEdges, class Visited>
struct try_edge<true, Edge, Target, AllEdges, Visited> {
private:
    using tail = typename find_path<
        typename Edge::destination_type,
        Target,
        AllEdges,
        type_list_append_t<Visited, typename Edge::destination_type>>::type;

public:
    using type = typename prepend_edge_to_path<
        Edge,
        tail,
        !std::is_same_v<tail, no_path>>::type;
};

template <class Current,
          class Target,
          class AllEdges,
          class Visited,
          class RemainingEdges>
struct scan_edges;

template <class Current, class Target, class AllEdges, class Visited>
struct scan_edges<Current,
                  Target,
                  AllEdges,
                  Visited,
                  transform_list<>> {
    using type = no_path;
};

template <bool Found,
          class Candidate,
          class Current,
          class Target,
          class AllEdges,
          class Visited,
          class... Remaining>
struct choose_path;

template <class Candidate,
          class Current,
          class Target,
          class AllEdges,
          class Visited,
          class... Remaining>
struct choose_path<true,
                   Candidate,
                   Current,
                   Target,
                   AllEdges,
                   Visited,
                   Remaining...> {
    using type = Candidate;
};

template <class Candidate,
          class Current,
          class Target,
          class AllEdges,
          class Visited,
          class... Remaining>
struct choose_path<false,
                   Candidate,
                   Current,
                   Target,
                   AllEdges,
                   Visited,
                   Remaining...> {
    using type = typename scan_edges<
        Current,
        Target,
        AllEdges,
        Visited,
        transform_list<Remaining...>>::type;
};

template <class Current,
          class Target,
          class AllEdges,
          class Visited,
          class Edge,
          class... Remaining>
struct scan_edges<Current,
                  Target,
                  AllEdges,
                  Visited,
                  transform_list<Edge, Remaining...>> {
private:
    static constexpr bool eligible =
        std::is_same_v<typename Edge::source_type, Current> &&
        !type_list_contains_v<typename Edge::destination_type, Visited>;

    using candidate =
        typename try_edge<eligible, Edge, Target, AllEdges, Visited>::type;

public:
    using type = typename choose_path<
        !std::is_same_v<candidate, no_path>,
        candidate,
        Current,
        Target,
        AllEdges,
        Visited,
        Remaining...>::type;
};

template <bool IsIdentity,
          class Current,
          class Target,
          class AllEdges,
          class Visited>
struct find_path_impl;

template <class Current, class Target, class AllEdges, class Visited>
struct find_path_impl<true, Current, Target, AllEdges, Visited> {
    using type = type_list<>;
};

template <class Current, class Target, class AllEdges, class Visited>
struct find_path_impl<false, Current, Target, AllEdges, Visited> {
    using type =
        typename scan_edges<Current, Target, AllEdges, Visited, AllEdges>::type;
};

template <class Current, class Target, class AllEdges, class Visited>
struct find_path
    : find_path_impl<std::is_same_v<Current, Target>,
                     Current,
                     Target,
                     AllEdges,
                     Visited> {};

template <class... Lists>
struct concatenate_transform_lists;

template <>
struct concatenate_transform_lists<> {
    using type = transform_list<>;
};

template <class... Specs, class... RemainingLists>
struct concatenate_transform_lists<transform_list<Specs...>,
                                   RemainingLists...> {
private:
    using tail =
        typename concatenate_transform_lists<RemainingLists...>::type;

    template <class>
    struct prepend_to_tail;

    template <class... TailSpecs>
    struct prepend_to_tail<transform_list<TailSpecs...>> {
        using type = transform_list<Specs..., TailSpecs...>;
    };

public:
    using type = typename prepend_to_tail<tail>::type;
};

template <class... Lists>
using concatenate_transform_lists_t =
    typename concatenate_transform_lists<Lists...>::type;

template <class>
inline constexpr bool dependent_false_v = false;

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
    using all_edges =
        concatenate_transform_lists_t<transformations_of_t<Callables>...>;

public:
    template <class... Fs>
    constexpr explicit composed_transform(Fs&&... callables)
        : callables_(std::forward<Fs>(callables)...) {}

    template <class Dst, class Src>
    constexpr auto to(Src&& source) {
        using source_type = std::remove_cvref_t<Src>;
        using path = typename find_path<
            source_type,
            Dst,
            all_edges,
            type_list<source_type>>::type;

        static_assert(!std::is_same_v<path, no_path>,
                      "No transformation path exists between these types");
        return apply_path(path{}, std::forward<Src>(source));
    }

    template <class Dst, class Src>
    constexpr auto to(Src&& source) const {
        using source_type = std::remove_cvref_t<Src>;
        using path = typename find_path<
            source_type,
            Dst,
            all_edges,
            type_list<source_type>>::type;

        static_assert(!std::is_same_v<path, no_path>,
                      "No transformation path exists between these types");
        return apply_path(path{}, std::forward<Src>(source));
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
    template <class Edge, std::size_t Index = 0>
    constexpr decltype(auto) callable_for() {
        if constexpr (Index == sizeof...(Callables)) {
            static_assert(dependent_false_v<Edge>,
                          "Internal error: no callable owns this edge");
        } else {
            using callable =
                std::tuple_element_t<Index, std::tuple<Callables...>>;
            if constexpr (transform_list_contains_v<
                              Edge,
                              transformations_of_t<callable>>) {
                return (std::get<Index>(callables_));
            } else {
                return callable_for<Edge, Index + 1>();
            }
        }
    }

    template <class Edge, std::size_t Index = 0>
    constexpr decltype(auto) callable_for() const {
        if constexpr (Index == sizeof...(Callables)) {
            static_assert(dependent_false_v<Edge>,
                          "Internal error: no callable owns this edge");
        } else {
            using callable =
                std::tuple_element_t<Index, std::tuple<Callables...>>;
            if constexpr (transform_list_contains_v<
                              Edge,
                              transformations_of_t<callable>>) {
                return (std::get<Index>(callables_));
            } else {
                return callable_for<Edge, Index + 1>();
            }
        }
    }

    template <class Edge, class Value>
    constexpr decltype(auto) apply_edge(Value&& value) {
        auto&& callable = callable_for<Edge>();
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
        auto&& callable = callable_for<Edge>();
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

    template <class Value>
    constexpr auto apply_path(type_list<>, Value&& value) {
        return std::remove_cvref_t<Value>(std::forward<Value>(value));
    }

    template <class Value>
    constexpr auto apply_path(type_list<>, Value&& value) const {
        return std::remove_cvref_t<Value>(std::forward<Value>(value));
    }

    template <class Edge, class... Remaining, class Value>
    constexpr auto apply_path(type_list<Edge, Remaining...>, Value&& value) {
        return apply_path(type_list<Remaining...>{},
                          apply_edge<Edge>(std::forward<Value>(value)));
    }

    template <class Edge, class... Remaining, class Value>
    constexpr auto apply_path(type_list<Edge, Remaining...>,
                              Value&& value) const {
        return apply_path(type_list<Remaining...>{},
                          apply_edge<Edge>(std::forward<Value>(value)));
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
