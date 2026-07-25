#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

#include "compose.hpp"

#ifndef GEOMETRY2_BENCHMARK_TYPE_COUNT
#error "GEOMETRY2_BENCHMARK_TYPE_COUNT must be defined"
#endif

#ifndef GEOMETRY2_BENCHMARK_USE_COUNT
#error "GEOMETRY2_BENCHMARK_USE_COUNT must be defined"
#endif

template <class Coordinate>
struct Tag {};

namespace {

inline constexpr std::size_t type_count = GEOMETRY2_BENCHMARK_TYPE_COUNT;
inline constexpr std::size_t use_count = GEOMETRY2_BENCHMARK_USE_COUNT;
inline constexpr std::size_t directed_pair_count = type_count * (type_count - 1);

static_assert(type_count >= 2);
static_assert(use_count >= 1);

template <std::size_t Index>
struct Coordinate {
    static_assert(Index < type_count);

    std::int64_t value;
};

template <std::size_t Pair>
inline constexpr std::size_t pair_source = Pair / (type_count - 1);

template <std::size_t Pair>
inline constexpr std::size_t pair_destination_slot = Pair % (type_count - 1);

template <std::size_t Pair>
inline constexpr std::size_t pair_destination =
    pair_destination_slot<Pair> +
    static_cast<std::size_t>(pair_destination_slot<Pair> >= pair_source<Pair>);

template <class PairSequence>
struct MakeTransformations;

template <std::size_t... Pairs>
struct MakeTransformations<std::index_sequence<Pairs...>> {
    using type = transform_list<
        transform_spec<
            Coordinate<pair_destination<Pairs>>,
            Coordinate<pair_source<Pairs>>>...>;
};

class CompleteTransform {
public:
    using transformations = typename MakeTransformations<
        std::make_index_sequence<directed_pair_count>>::type;

    template <std::size_t Destination, std::size_t Source>
        requires (
            Destination < type_count &&
            Source < type_count &&
            Destination != Source)
    constexpr Coordinate<Destination> operator()(
        const Coordinate<Source>& coordinate,
        Tag<Coordinate<Destination>>) const
    {
        return {
            coordinate.value +
            static_cast<std::int64_t>(Destination) -
            static_cast<std::int64_t>(Source)};
    }
};

inline auto composed_transforms = compose_transforms(CompleteTransform{});

template <std::size_t Use, std::size_t Pair>
std::int64_t exercise_pair()
{
    const Coordinate<pair_source<Pair>> source{
        static_cast<std::int64_t>(Use * directed_pair_count + Pair + 1)};

    return composed_transforms
        .template to<Coordinate<pair_destination<Pair>>>(source)
        .value;
}

template <std::size_t Use, std::size_t... Pairs>
std::int64_t exercise_all_pairs(std::index_sequence<Pairs...>)
{
    return (std::int64_t{} + ... + exercise_pair<Use, Pairs>());
}

template <std::size_t... Uses>
std::int64_t exercise_all_uses(std::index_sequence<Uses...>)
{
    return (
        std::int64_t{} + ... +
        exercise_all_pairs<Uses>(
            std::make_index_sequence<directed_pair_count>{}));
}

inline const std::int64_t workload_checksum = exercise_all_uses(
    std::make_index_sequence<use_count>{});

} // namespace

int main()
{
    return workload_checksum == 0;
}
