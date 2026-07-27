#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <utility>

#include "spatia/transforms/combine.hpp"

using namespace spatia;

namespace {

inline constexpr std::size_t chain_length = 10;

template <std::size_t Index>
struct Coordinate {
    std::uint64_t value;
};

template <std::size_t Index>
struct AddStep {
    using Source = Coordinate<Index>;
    using Destination = Coordinate<Index + 1>;
    using Transformations = TransformList<TransformSpec<Destination, Source>>;

    constexpr Destination operator()(const Source& source) const noexcept { return {source.value + Index + 1}; }
};

template <std::size_t... Indices>
auto make_chain(std::index_sequence<Indices...>) {
    return combine(AddStep<Indices>{}...);
}

template <std::size_t Index>
constexpr auto run_manual_chain(Coordinate<Index> source) noexcept {
    if constexpr (Index == chain_length) {
        return source;
    } else {
        return run_manual_chain<Index + 1>(AddStep<Index>{}(source));
    }
}

constexpr std::uint64_t collapsed_delta = chain_length * (chain_length + 1) / 2;

static_assert(collapsed_delta == 55);

void record_transforms(benchmark::State& state) {
    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(chain_length));
}

void BM_ComposedChain10(benchmark::State& state) {
    const auto chain = make_chain(std::make_index_sequence<chain_length>{});
    Coordinate<0> source{1};

    const auto probe = chain.to<Coordinate<chain_length>>(source);
    if (probe.value != source.value + collapsed_delta) {
        state.SkipWithError("composed chain returned an invalid value");
        return;
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(source);
        auto result = chain.to<Coordinate<chain_length>>(source);
        benchmark::DoNotOptimize(result);
        source.value = result.value;
    }

    record_transforms(state);
}

void BM_ManualChain10(benchmark::State& state) {
    Coordinate<0> source{1};

    for (auto _ : state) {
        benchmark::DoNotOptimize(source);
        auto result = run_manual_chain(source);
        benchmark::DoNotOptimize(result);
        source.value = result.value;
    }

    record_transforms(state);
}

void BM_CollapsedChain10(benchmark::State& state) {
    Coordinate<0> source{1};

    for (auto _ : state) {
        benchmark::DoNotOptimize(source);
        Coordinate<chain_length> result{source.value + collapsed_delta};
        benchmark::DoNotOptimize(result);
        source.value = result.value;
    }

    record_transforms(state);
}

BENCHMARK(BM_ComposedChain10)->Unit(benchmark::kNanosecond);
BENCHMARK(BM_ManualChain10)->Unit(benchmark::kNanosecond);
BENCHMARK(BM_CollapsedChain10)->Unit(benchmark::kNanosecond);

}  // namespace
