#include "compose.hpp"

#include <utility>

template <class T>
struct Tag {};

template <int Group, int Index>
struct Node {
    int value{};
};

template <class Left, class Right>
struct BidirectionalEdge {
    using transformations = transform_list<
        transform_spec<Right, Left>,
        transform_spec<Left, Right>>;

    Right operator()(const Left& value, Tag<Right>) const {
        return Right{value.value};
    }

    Left operator()(const Right& value, Tag<Left>) const {
        return Left{value.value};
    }
};

template <int Group, int Left, int Right>
using Edge =
    BidirectionalEdge<Node<Group, Left>, Node<Group, Right>>;

#define TREE(Group)                                                            \
    Edge<Group, 0, 1>{}, Edge<Group, 0, 2>{}, Edge<Group, 1, 3>{},             \
        Edge<Group, 1, 4>{}, Edge<Group, 2, 5>{}, Edge<Group, 2, 6>{},         \
        Edge<Group, 3, 7>{}, Edge<Group, 3, 8>{}, Edge<Group, 4, 9>{}

using BenchmarkComposition = decltype(
    compose_transforms(TREE(0), TREE(1), TREE(2), TREE(3)));

#undef TREE

template <int Group, int Source, int... Destinations>
void instantiate_row(
    BenchmarkComposition& composition,
    std::integer_sequence<int, Destinations...>) {
    Node<Group, Source> source{};
    ((void)composition.template to<Node<Group, Destinations>>(source), ...);
}

template <int Group, int... Sources>
void instantiate_group(
    BenchmarkComposition& composition,
    std::integer_sequence<int, Sources...>) {
    (instantiate_row<Group, Sources>(
         composition,
         std::make_integer_sequence<int, 10>{}),
     ...);
}

// Instantiates all 100 source/destination pairs in each of four disconnected
// ten-node trees: 400 conversions in total.
void benchmark_compile_time(BenchmarkComposition& composition) {
    instantiate_group<0>(
        composition,
        std::make_integer_sequence<int, 10>{});
    instantiate_group<1>(
        composition,
        std::make_integer_sequence<int, 10>{});
    instantiate_group<2>(
        composition,
        std::make_integer_sequence<int, 10>{});
    instantiate_group<3>(
        composition,
        std::make_integer_sequence<int, 10>{});
}
