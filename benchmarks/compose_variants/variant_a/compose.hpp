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
template <class>
struct callable_traits;

template <class R, class FirstArg, class... Rest>
struct callable_traits<R(FirstArg, Rest...)> {
    static_assert(
        sizeof...(Rest) <= 1,
        "A transform callable must accept SRC or SRC, Tag<DST>");

    using source_type = std::remove_cvref_t<FirstArg>;
    using destination_type = std::remove_cvref_t<R>;
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

// Free function pointer (compose_transforms decays function references to this).
template <class T>
struct inferred_callable_traits<
    T,
    std::enable_if_t<std::is_pointer_v<T> && std::is_function_v<std::remove_pointer_t<T>>>>
    : callable_traits<T> {};

// A class with one non-overloaded operator(), including an ordinary lambda.
template <class T>
struct inferred_callable_traits<T, std::void_t<decltype(&T::operator())>>
    : callable_traits<decltype(&T::operator())> {};

// Converts the public transform_list into a tuple used internally.
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

    using type = std::tuple<Specs...>;
};

template <class Transform>
concept has_declared_transformations = requires {
    typename std::remove_cvref_t<Transform>::transformations;
};

template <class Transform, bool = has_declared_transformations<Transform>>
struct transform_specs;

// An overloaded functor declares all supported edges explicitly.
template <class Transform>
struct transform_specs<Transform, true> {
private:
    using declared = typename std::remove_cvref_t<Transform>::transformations;

public:
    using type = typename normalize_transform_list<declared>::type;
};

// A free function, function pointer, or non-overloaded functor/lambda is inferred.
template <class Transform>
struct transform_specs<Transform, false> {
private:
    using traits = inferred_callable_traits<std::remove_cvref_t<Transform>>;

public:
    using type = std::tuple<transform_spec<
        typename traits::destination_type,
        typename traits::source_type>>;
};

template <std::size_t OwnerIndex, class Spec>
struct bound_edge {
    static constexpr std::size_t owner_index = OwnerIndex;
    using source_type = typename Spec::source_type;
    using destination_type = typename Spec::destination_type;
};

template <std::size_t OwnerIndex, class SpecsTuple>
struct bind_edges;

template <std::size_t OwnerIndex, class... Specs>
struct bind_edges<OwnerIndex, std::tuple<Specs...>> {
    using type = std::tuple<bound_edge<OwnerIndex, Specs>...>;
};

template <class Left, class Right>
struct tuple_concat;

template <class... Left, class... Right>
struct tuple_concat<std::tuple<Left...>, std::tuple<Right...>> {
    using type = std::tuple<Left..., Right...>;
};

template <class Left, class Right>
using tuple_concat_t = typename tuple_concat<Left, Right>::type;

template <
    class TransformsTuple,
    std::size_t I,
    bool End = (I == std::tuple_size_v<TransformsTuple>)>
struct collect_edges_impl;

template <class TransformsTuple, std::size_t I>
struct collect_edges_impl<TransformsTuple, I, true> {
    using type = std::tuple<>;
};

template <class TransformsTuple, std::size_t I>
struct collect_edges_impl<TransformsTuple, I, false> {
private:
    using transform_type = std::tuple_element_t<I, TransformsTuple>;
    using specs = typename transform_specs<transform_type>::type;
    using current_edges = typename bind_edges<I, specs>::type;
    using remaining_edges = typename collect_edges_impl<TransformsTuple, I + 1>::type;

public:
    using type = tuple_concat_t<current_edges, remaining_edges>;
};

template <class TransformsTuple>
using collect_edges_t = typename collect_edges_impl<TransformsTuple, 0>::type;

template <
    class EdgesTuple,
    class Current,
    class Destination,
    class Visited>
struct has_path;

template <
    class EdgesTuple,
    std::size_t I,
    class Current,
    class Destination,
    class Visited>
struct has_path_through_edge;

template <
    class EdgesTuple,
    class Current,
    class Destination,
    class Visited>
struct has_path {
private:
    static consteval bool calculate()
    {
        if constexpr (std::same_as<Current, Destination>) {
            return true;
        } else if constexpr (contains_v<Current, Visited>) {
            return false;
        } else {
            using next_visited = push_front_t<Current, Visited>;
            return has_path_through_edge<
                EdgesTuple,
                0,
                Current,
                Destination,
                next_visited>::value;
        }
    }

public:
    static constexpr bool value = calculate();
};

template <
    class EdgesTuple,
    std::size_t I,
    class Current,
    class Destination,
    class Visited>
struct has_path_through_edge {
private:
    static consteval bool calculate()
    {
        if constexpr (I == std::tuple_size_v<EdgesTuple>) {
            return false;
        } else {
            using edge_type = std::tuple_element_t<I, EdgesTuple>;
            using edge_source = typename edge_type::source_type;
            using edge_destination = typename edge_type::destination_type;

            if constexpr (
                std::same_as<Current, edge_source> &&
                has_path<
                    EdgesTuple,
                    edge_destination,
                    Destination,
                    Visited>::value) {
                return true;
            } else {
                return has_path_through_edge<
                    EdgesTuple,
                    I + 1,
                    Current,
                    Destination,
                    Visited>::value;
            }
        }
    }

public:
    static constexpr bool value = calculate();
};

template <class EdgesTuple, class Source, class Destination>
inline constexpr bool has_path_v = has_path<
    EdgesTuple,
    Source,
    Destination,
    type_list<>>::value;

// Prefer the tagged form because it disambiguates an overloaded operator().
template <class Destination, class Callable, class Source>
constexpr Destination invoke_transform(const Callable& callable, const Source& source)
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

    constexpr Destination operator()(const Source& source, Tag<Destination> = {}) const
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
    using edges_tuple = compose_detail::collect_edges_t<transforms_tuple>;

public:
    template <class... Ts>
        requires (sizeof...(Ts) == sizeof...(Transforms))
    explicit constexpr ComposedTransforms(Ts&&... transforms)
        : transforms_(std::forward<Ts>(transforms)...)
    {}

    template <class Destination, class Source>
        requires compose_detail::has_path_v<
            edges_tuple,
            std::remove_cvref_t<Source>,
            Destination>
    constexpr Destination operator()(
        const Source& source,
        Tag<Destination> = {}) const
    {
        using source_type = std::remove_cvref_t<Source>;
        return convert<Destination, source_type, compose_detail::type_list<>>(source);
    }

    // Optional compatibility/convenience aliases.
    template <class Destination, class Source>
        requires compose_detail::has_path_v<
            edges_tuple,
            std::remove_cvref_t<Source>,
            Destination>
    constexpr Destination transform(
        const Source& source,
        Tag<Destination> tag = {}) const
    {
        return (*this)(source, tag);
    }

    template <class Destination, class Source>
        requires compose_detail::has_path_v<
            edges_tuple,
            std::remove_cvref_t<Source>,
            Destination>
    constexpr Destination to(const Source& source) const
    {
        return (*this)(source, Tag<Destination>{});
    }

private:
    template <class Destination, class Current, class Visited>
    constexpr Destination convert(const Current& current) const
    {
        if constexpr (std::same_as<Current, Destination>) {
            return current;
        } else {
            static_assert(!compose_detail::contains_v<Current, Visited>);
            using next_visited = compose_detail::push_front_t<Current, Visited>;

            return convert_through_edge<
                0,
                Destination,
                Current,
                next_visited>(current);
        }
    }

    template <
        std::size_t I,
        class Destination,
        class Current,
        class Visited>
    constexpr Destination convert_through_edge(const Current& current) const
    {
        if constexpr (I == std::tuple_size_v<edges_tuple>) {
            static_assert(
                compose_detail::always_false_v<Destination, Current, Visited>,
                "No transform path found");
        } else {
            using edge_type = std::tuple_element_t<I, edges_tuple>;
            using edge_source = typename edge_type::source_type;
            using edge_destination = typename edge_type::destination_type;

            if constexpr (
                std::same_as<Current, edge_source> &&
                compose_detail::has_path<
                    edges_tuple,
                    edge_destination,
                    Destination,
                    Visited>::value) {
                const auto& transformer =
                    std::get<edge_type::owner_index>(transforms_);

                const auto next =
                    compose_detail::invoke_transform<edge_destination>(
                        transformer,
                        current);

                return convert<Destination, edge_destination, Visited>(next);
            } else {
                return convert_through_edge<
                    I + 1,
                    Destination,
                    Current,
                    Visited>(current);
            }
        }
    }

    transforms_tuple transforms_;
};

template <class... Transforms>
constexpr auto compose_transforms(Transforms&&... transforms)
{
    return ComposedTransforms<std::decay_t<Transforms>...>{
        std::forward<Transforms>(transforms)...};
}