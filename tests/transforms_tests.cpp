#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cmath>

#include "spatia/transforms/combine.hpp"
#include "spatia/transforms/compose.hpp"

using namespace spatia;

struct A {
    int x;
    bool operator==(const A& rhs) const = default;
};

struct B {
    int x;
    bool operator==(const B& rhs) const = default;
};

struct C {
    int x;
    bool operator==(const C& rhs) const = default;
};

struct D {
    int x;
    bool operator==(const D& rhs) const = default;
};

template <class DST, class SRC>
class Transform {
   public:
    int delta;

    DST operator()(const SRC& p, Tag<DST> = {}) const { return DST{p.x + delta}; }
};

TEST(ComposeTest, SingleTransform) {
    Transform<B, A> t_b_from_a{10};
    // static_assert(TransformDst<B, Transform<B, A>, A>);
    // static_assert(TransformSrc<A, Transform<B, A>, B>);
    A p_a{1};
    B p_b{10 + 1};
    EXPECT_EQ(t_b_from_a(p_a), p_b);
    EXPECT_TRUE(true);  // dummy assertion to avoid "no tests" warning
}

TEST(ComposeTest, HandlesSingleAndIdentity) {
    Transform<B, A> t_b_from_a{10};
    A p_a{1};
    B p_b{10 + 1};
    auto t_multi = combine(t_b_from_a);
    EXPECT_EQ(t_multi.to<B>(p_a), p_b);
    EXPECT_EQ(t_multi.to<A>(p_a), p_a);
    EXPECT_EQ(t_multi.to<B>(p_b), p_b);
}

TEST(ComposeTest, HandlesComposingThereAndBack) {
    Transform<B, A> t_b_from_a{10};
    Transform<A, B> t_a_from_b{-10};
    A p_a{1};
    B p_b{10 + 1};

    auto t_multi = combine(t_b_from_a, t_a_from_b);
    EXPECT_EQ(t_multi.to<B>(p_a), p_b);
    EXPECT_EQ(t_multi.to<A>(p_b), p_a);
}

TEST(ComposeTest, HandlesAllTransformsWhenComposingTwo) {
    Transform<B, A> t_b_from_a{10};
    Transform<C, B> t_c_from_b{100};
    A p_a{1};
    B p_b{10 + 1};
    C p_c{100 + 10 + 1};

    // Composed<Transform<B, A>, Transform<C, B>> t_multi2;
    auto t_multi = combine(t_c_from_b, t_b_from_a);
    EXPECT_EQ(p_b, t_multi.to<B>(p_a));
    EXPECT_EQ(p_c, t_multi.to<C>(p_a));
    EXPECT_EQ(p_c, t_multi.to<C>(p_b));
}

TEST(ComposeTest, HandlesAllTransformsWhenComposingTwoInDifferentOrder) {
    Transform<B, A> t_b_from_a{10};
    Transform<C, B> t_c_from_b{100};
    A p_a{1};
    B p_b{10 + 1};
    C p_c{100 + 10 + 1};

    auto t_multi = combine(t_b_from_a, t_c_from_b);
    EXPECT_EQ(p_b, t_multi.to<B>(p_a));
    EXPECT_EQ(p_c, t_multi.to<C>(p_a));
    EXPECT_EQ(p_c, t_multi.to<C>(p_b));
}

namespace {
struct ThreeTransformChainFixture {
    Transform<B, A> t_b_from_a{10};
    Transform<C, B> t_c_from_b{100};
    Transform<D, C> t_d_from_c{1000};
    A p_a{1};
    B p_b{10 + 1};
    C p_c{100 + 10 + 1};
    D p_d{1000 + 100 + 10 + 1};

    template <class COMPOSED>
    void expect_all_paths(const COMPOSED& t_multi) const {
        EXPECT_EQ(p_b, t_multi.template to<B>(p_a));
        EXPECT_EQ(p_c, t_multi.template to<C>(p_a));
        EXPECT_EQ(p_c, t_multi.template to<C>(p_b));
        EXPECT_EQ(p_d, t_multi.template to<D>(p_a));
        EXPECT_EQ(p_d, t_multi.template to<D>(p_b));
        EXPECT_EQ(p_d, t_multi.template to<D>(p_c));
    }
};
}  // namespace

TEST(ComposeTest, HandlesAllTransformsWhenComposingThree) {
    ThreeTransformChainFixture fixture;

    auto t_multi = combine(fixture.t_d_from_c, fixture.t_c_from_b, fixture.t_b_from_a);
    fixture.expect_all_paths(t_multi);
}

TEST(ComposeTest, HandlesAllTransformsWhenComposingThreeInReverseOrder) {
    ThreeTransformChainFixture fixture;

    auto t_multi = combine(fixture.t_b_from_a, fixture.t_c_from_b, fixture.t_d_from_c);
    fixture.expect_all_paths(t_multi);
}

TEST(ComposeTest, HandlesAllTransformsWhenComposingThreeInDifferentOrder) {
    ThreeTransformChainFixture fixture;
    auto t_multi = combine(fixture.t_b_from_a, fixture.t_d_from_c, fixture.t_c_from_b);
    fixture.expect_all_paths(t_multi);
}

TEST(ComposeTest, HandlesAllTransformsWhenComposingThreeInDifferentOrder2) {
    ThreeTransformChainFixture fixture;

    // any order but must introduce one new cs at a time (cannot connect two
    // already existing ones)
    auto t_multi = combine(fixture.t_d_from_c, fixture.t_b_from_a, fixture.t_c_from_b);
    fixture.expect_all_paths(t_multi);
}

TEST(ComposeTest, HandlesAllTransformsWhenComposingThreeWithInverses) {
    Transform<B, A> t_b_from_a{10};     // B <- A
    Transform<C, B> t_c_from_b{100};    // C <- B
    Transform<D, C> t_d_from_c{1000};   // D <- C
    Transform<A, B> t_a_from_b{-10};    // A <- B
    Transform<B, C> t_b_from_c{-100};   // B <- C
    Transform<C, D> t_c_from_d{-1000};  // C <- D

    A p_a{1};
    B p_b{10 + 1};
    C p_c{100 + 10 + 1};
    D p_d{1000 + 100 + 10 + 1};

    auto t_multi = combine(t_b_from_a, t_a_from_b, t_b_from_c, t_c_from_b, t_c_from_d, t_d_from_c);

    EXPECT_EQ(p_a, t_multi.to<A>(p_a));
    EXPECT_EQ(p_b, t_multi.to<B>(p_a));
    EXPECT_EQ(p_c, t_multi.to<C>(p_a));
    EXPECT_EQ(p_d, t_multi.to<D>(p_a));
    EXPECT_EQ(p_a, t_multi.to<A>(p_b));
    EXPECT_EQ(p_b, t_multi.to<B>(p_b));
    EXPECT_EQ(p_c, t_multi.to<C>(p_b));
    EXPECT_EQ(p_d, t_multi.to<D>(p_b));
    EXPECT_EQ(p_a, t_multi.to<A>(p_c));
    EXPECT_EQ(p_b, t_multi.to<B>(p_c));
    EXPECT_EQ(p_c, t_multi.to<C>(p_c));
    EXPECT_EQ(p_d, t_multi.to<D>(p_c));
    EXPECT_EQ(p_a, t_multi.to<A>(p_d));
    EXPECT_EQ(p_b, t_multi.to<B>(p_d));
    EXPECT_EQ(p_c, t_multi.to<C>(p_d));
    EXPECT_EQ(p_d, t_multi.to<D>(p_d));
}

template <class DST, class SRC>
class BiTransform {
   public:
    int delta;
    DST operator()(const SRC& p, Tag<DST> = {}) const { return DST{p.x + delta}; }
    SRC operator()(const DST& p, Tag<SRC> = {}) const { return SRC{p.x - delta}; }

    using Transformations = TransformList<TransformSpec<SRC, DST>, TransformSpec<DST, SRC> >;
};

TEST(ComposeTest, HandlesBidirectionalTransforms) {
    Transform<B, A> t_b_from_a{10};
    Transform<A, B> t_a_from_b{-10};
    BiTransform<C, B> t_b_c{100};
    A p_a{1};
    B p_b{10 + 1};
    C p_c{100 + 10 + 1};

    auto t_multi = combine(t_a_from_b, t_b_c, t_b_from_a);

    EXPECT_EQ(p_a, t_multi.to<A>(p_a));
    EXPECT_EQ(p_b, t_multi.to<B>(p_a));
    EXPECT_EQ(p_c, t_multi.to<C>(p_a));
    EXPECT_EQ(p_a, t_multi.to<A>(p_b));
    EXPECT_EQ(p_b, t_multi.to<B>(p_b));
    EXPECT_EQ(p_c, t_multi.to<C>(p_b));
    EXPECT_EQ(p_a, t_multi.to<A>(p_c));
    EXPECT_EQ(p_b, t_multi.to<B>(p_c));
    EXPECT_EQ(p_c, t_multi.to<C>(p_c));
}

// A non-overloaded ordinary function is inferred automatically.
B free_b_from_a(const A& p) { return B{p.x + 10}; }

TEST(ComposeTest, SupportsOrdinaryFunctionAndCapturingLambda) {
    constexpr int delta = 100;
    auto c_from_b = [](const B& p) { return C{p.x + delta}; };

    // A function name and a lambda can be composed directly.
    auto t_multi = combine(free_b_from_a, c_from_b);

    EXPECT_EQ(t_multi(A{1}, Tag<B>{}), B{11});
    EXPECT_EQ(t_multi(A{1}, Tag<C>{}), C{111});
}

// An overloaded function object declares the graph edges it exposes.
class OverloadedFunctor {
   public:
    using Transformations = TransformList<TransformSpec<A, C>,  // C -> A
                                          TransformSpec<B, C>   // C -> B
                                          >;

    A operator()(const C& p, Tag<A>) const { return A{p.x + 1000}; }

    B operator()(const C& p, Tag<B>) const { return B{p.x + 100}; }
};

TEST(ComposeTest, SupportsOverloadedFunctionObject) {
    Transform<C, D> c_from_d{10};
    OverloadedFunctor overloaded;

    auto t_multi = combine(overloaded, c_from_d);

    EXPECT_EQ(t_multi(D{1}, Tag<A>{}), A{1011});
    EXPECT_EQ(t_multi(D{1}, Tag<B>{}), B{111});
}

// Several overloads of one ordinary function must be selected one by one.
A overloaded_free(const C& p, Tag<A>) { return A{p.x + 1000}; }

B overloaded_free(const C& p, Tag<B>) { return B{p.x + 100}; }

D overloaded_free(const B& p, Tag<D>) { return D{p.x + 10'000}; }

TEST(ComposeTest, SupportsAllSelectedOverloadsOfOrdinaryFunction) {
    Transform<C, D> c_from_d{10};

    auto t_multi = combine(
        // Each expression below is a distinct function pointer and therefore
        // a distinct edge in the composed type graph.
        overload_cast<A(const C&, Tag<A>)>(overloaded_free), overload_cast<B(const C&, Tag<B>)>(overloaded_free),
        overload_cast<D(const B&, Tag<D>)>(overloaded_free), c_from_d);

    EXPECT_EQ(t_multi(D{1}, Tag<A>{}), A{1011});    // D -> C -> A
    EXPECT_EQ(t_multi(D{1}, Tag<B>{}), B{111});     // D -> C -> B
    EXPECT_EQ(t_multi(C{1}, Tag<D>{}), D{10'101});  // C -> B -> D
}

TEST(ComposeTest, SupportsGenericLambdaWithExplicitMetadata) {
    const int delta = 100;
    auto generic = [delta](const auto& p) { return C{p.x + delta}; };

    auto c_from_b = make_transform<C, B>(generic);
    auto t_multi = combine(Transform<B, A>{10}, c_from_b);

    EXPECT_EQ(t_multi(A{1}, Tag<C>{}), C{111});
}

TEST(ComposeTest, ComposesTwoArbitraryTransformsIntoOne) {
    const auto b_from_a = [](const A& p) { return B{p.x + 10}; };
    const auto c_from_b = [](const B& p) { return C{p.x + 100}; };

    const auto c_from_a = c_from_b * b_from_a;

    EXPECT_EQ(c_from_a(A{1}), C{111});
    EXPECT_EQ(c_from_a(A{1}, Tag<C>{}), C{111});

    // The composition is itself a transform, so it can be composed further
    // and fed to combine().
    const auto d_from_c = [](const C& p) { return D{p.x + 1000}; };
    EXPECT_EQ((d_from_c * c_from_a)(A{1}), D{1111});
    EXPECT_EQ(combine(c_from_a, d_from_c).to<D>(A{1}), D{1111});
}
