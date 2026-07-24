#include <gtest/gtest.h>


// Helper matcher for comparing points with tolerance
#include <cmath>
#include "common.hpp"
#include "compose.hpp"
#include "view.hpp"
#include <gmock/gmock.h>


struct CsA {
};

struct CsB {
};

struct CsC {
};

struct CsD {
};



TEST(Geometry2Tests, SingleTransform) {
    Transform<CsB, CsA> t_b_from_a{10};
    static_assert(TransformDst<Point<CsB>, Transform<CsB, CsA>, Point<CsA>>);
    static_assert(TransformSrc<Point<CsA>, Transform<CsB, CsA>, Point<CsB>>);
    Point<CsA> p_a{1};
    Point<CsB> p_b{10 + 1};
    EXPECT_EQ(t_b_from_a.transform(p_a), p_b);
    EXPECT_TRUE(true); // dummy assertion to avoid "no tests" warning

}

// TODO: Transform Concept
// concept IsTransform = requires(const auto& t, const auto& p, const auto& dummy_dst) {


TEST(ComposedTest, HandlesSingleAndIdentity) {
    Transform<CsB, CsA> t_b_from_a{10};
    Point<CsA> p_a{1};
    Point<CsB> p_b{10 + 1};
    auto t_multi = compose_transforms(t_b_from_a);
    EXPECT_EQ(t_multi.to<Point<CsB>>(p_a), p_b);
    EXPECT_EQ(t_multi.to<Point<CsA>>(p_a), p_a);
    EXPECT_EQ(t_multi.to<Point<CsB>>(p_b), p_b);

}

TEST(ComposedTest, HandlesComposingThereAndBack) {
    Transform<CsB, CsA> t_b_from_a{10};
    Transform<CsA, CsB> t_a_from_b{-10};
    Point<CsA> p_a{1};
    Point<CsB> p_b{10 + 1};

    auto t_multi = compose_transforms(t_b_from_a, t_a_from_b);
    EXPECT_EQ(t_multi.to<Point<CsB>>(p_a), p_b);
    EXPECT_EQ(t_multi.to<Point<CsA>>(p_b), p_a);
}

TEST(ComposedTest, HandlesAllTransformsWhenComposingTwo) {
    Transform<CsB, CsA> t_b_from_a{10};
    Transform<CsC, CsB> t_c_from_b{100};
    Point<CsA> p_a{1};
    Point<CsB> p_b{10 + 1};
    Point<CsC> p_c{100 + 10 + 1};
    
    // Composed<Transform<CsB, CsA>, Transform<CsC, CsB>> t_multi2;
    auto t_multi = compose_transforms(t_c_from_b, t_b_from_a);
    EXPECT_EQ(p_b, t_multi.to<Point<CsB>>(p_a));
    EXPECT_EQ(p_c, t_multi.to<Point<CsC>>(p_a));
    EXPECT_EQ(p_c, t_multi.to<Point<CsC>>(p_b));
}

TEST(ComposedTest, HandlesAllTransformsWhenComposingTwoInDifferentOrder) {
    Transform<CsB, CsA> t_b_from_a{10};
    Transform<CsC, CsB> t_c_from_b{100};
    Point<CsA> p_a{1};
    Point<CsB> p_b{10 + 1};
    Point<CsC> p_c{100 + 10 + 1};
    
    auto t_multi = compose_transforms(t_b_from_a, t_c_from_b);
    EXPECT_EQ(p_b, t_multi.to<Point<CsB>>(p_a));
    EXPECT_EQ(p_c, t_multi.to<Point<CsC>>(p_a));
    EXPECT_EQ(p_c, t_multi.to<Point<CsC>>(p_b));
}

namespace {
struct ThreeTransformChainFixture {
    Transform<CsB, CsA> t_b_from_a{10};
    Transform<CsC, CsB> t_c_from_b{100};
    Transform<CsD, CsC> t_d_from_c{1000};
    Point<CsA> p_a{1};
    Point<CsB> p_b{10 + 1};
    Point<CsC> p_c{100 + 10 + 1};
    Point<CsD> p_d{1000 + 100 + 10 + 1};

    template <class COMPOSED>
    void expect_all_paths(const COMPOSED& t_multi) const {
        EXPECT_EQ(p_b, t_multi.template to<Point<CsB>>(p_a));
        EXPECT_EQ(p_c, t_multi.template to<Point<CsC>>(p_a));
        EXPECT_EQ(p_c, t_multi.template to<Point<CsC>>(p_b));
        EXPECT_EQ(p_d, t_multi.template to<Point<CsD>>(p_a));
        EXPECT_EQ(p_d, t_multi.template to<Point<CsD>>(p_b));
        EXPECT_EQ(p_d, t_multi.template to<Point<CsD>>(p_c));
    }
};
} // namespace

TEST(ComposedTest, HandlesAllTransformsWhenComposingThree) {
    ThreeTransformChainFixture fixture;

    auto t_multi =
        compose_transforms(fixture.t_d_from_c, fixture.t_c_from_b, fixture.t_b_from_a);
    fixture.expect_all_paths(t_multi);
}

TEST(ComposedTest, HandlesAllTransformsWhenComposingThreeInReverseOrder) {
    ThreeTransformChainFixture fixture;

    auto t_multi =
        compose_transforms(fixture.t_b_from_a, fixture.t_c_from_b, fixture.t_d_from_c);
    fixture.expect_all_paths(t_multi);
}

TEST(ComposedTest, HandlesAllTransformsWhenComposingThreeInDifferentOrder) {
    ThreeTransformChainFixture fixture;
    auto t_multi =
        compose_transforms(fixture.t_b_from_a, fixture.t_d_from_c, fixture.t_c_from_b);
    fixture.expect_all_paths(t_multi);
}

TEST(ComposedTest, HandlesAllTransformsWhenComposingThreeInDifferentOrder2) {
    ThreeTransformChainFixture fixture;

    // any order but must introduce one new cs at a time (cannot connect two already existing ones)
    auto t_multi =
        compose_transforms(fixture.t_d_from_c, fixture.t_b_from_a, fixture.t_c_from_b);
    fixture.expect_all_paths(t_multi);
}

TEST(ComposedTest, HandlesAllTransformsWhenComposingThreeWithInverses)
{
    Transform<CsB, CsA> t_b_from_a{10};
    Transform<CsC, CsB> t_c_from_b{100};
    Transform<CsD, CsC> t_d_from_c{1000};
    Transform<CsA, CsB> t_a_from_b{-10};
    Transform<CsB, CsC> t_b_from_c{-100};
    Transform<CsC, CsD> t_c_from_d{-1000};
    Point<CsA> p_a{1};
    Point<CsB> p_b{10 + 1};
    Point<CsC> p_c{100 + 10 + 1};
    Point<CsD> p_d{1000 + 100 + 10 + 1};

    auto t_multi = compose_transforms(t_a_from_b, t_b_from_c, t_c_from_b, t_c_from_d, t_d_from_c);
    // auto t_multi = compose_transforms(t_a_from_b, t_b_from_a, t_b_from_c, t_c_from_b, t_c_from_d, t_d_from_c);

    // EXPECT_EQ(p_a, t_multi.to<Point<CsA>>(p_a));
    // EXPECT_EQ(p_b, t_multi.to<Point<CsB>>(p_a));
    // EXPECT_EQ(p_c, t_multi.to<Point<CsC>>(p_a));
    // EXPECT_EQ(p_d, t_multi.to<Point<CsD>>(p_a));
    // EXPECT_EQ(p_a, t_multi.to<Point<CsA>>(p_b));
    EXPECT_EQ(p_b, t_multi.to<Point<CsB>>(p_b));
    EXPECT_EQ(p_c, t_multi.to<Point<CsC>>(p_b));
    EXPECT_EQ(p_d, t_multi.to<Point<CsD>>(p_b));
    // EXPECT_EQ(p_a, t_multi.to<Point<CsA>>(p_c));
    EXPECT_EQ(p_b, t_multi.to<Point<CsB>>(p_c));
    EXPECT_EQ(p_c, t_multi.to<Point<CsC>>(p_c));
    EXPECT_EQ(p_d, t_multi.to<Point<CsD>>(p_c));
    // EXPECT_EQ(p_a, t_multi.to<Point<CsA>>(p_d));
    EXPECT_EQ(p_b, t_multi.to<Point<CsB>>(p_d));  // d->c->b t_b_from_c(t_c_from_d(p_d, Tag<C>), Tag<B>)
    auto tt = t_multi.transformable_from(p_d);

    EXPECT_EQ(p_c, t_multi.to<Point<CsC>>(p_d));
    EXPECT_EQ(p_d, t_multi.to<Point<CsD>>(p_d));
}


void func(const TransformsView<CsD, CsC, CsB> &t)
{
    auto p_b = Point<CsB>{10};
    auto p_c = Point<CsC>{100 + 10};
    auto p_d = Point<CsD>{1000 + 100 + 10};

    EXPECT_EQ(t.to<Point<CsC>>(p_b), p_c);
    EXPECT_EQ(t.to<Point<CsD>>(p_b), p_d);
    EXPECT_EQ(t.to<Point<CsD>>(p_c), p_d);
}


TEST(TransformsViewTest, CanBePassedToFunctions) {
    Transform<CsB, CsA> t_b_from_a{10};
    Transform<CsC, CsB> t_c_from_b{100};
    Transform<CsD, CsC> t_d_from_c{1000};

    auto t_multi = compose_transforms(t_d_from_c, t_c_from_b, t_b_from_a);
    func(t_multi);
}




#if 0
template<class...> 
class ITransformsTo;


template<class BASE, class DST> 
class ITransformsTo<BASE, DST>: public BASE
{
public:
    using BASE::transform;
};

/// Provides transform() for SRCS->DST, i.e.: SRC->DST SRCS[0]->DST, SRCS[1]->DST, ...
template<class BASE, class DST, class SRC, class ... SRCS> 
class ITransformsTo<BASE, DST, SRC, SRCS...>: public ITransformsTo<BASE, DST, SRCS...>
{
public:
    using ITransformsTo<BASE, DST, SRCS...>::transform;
    virtual Point<DST> transform(const Point<SRC>& p, Tag<Point<DST>> dummy_dst={}) const = 0;

};


template<class...>
class ITransformsA2A;

template<class DST>
class ITransformsA2A<DST> {
public:
    void transform();
    virtual ~ITransformsA2A() = default;

};

/// Provides transform() for SRCS[1:] -> SRCS[0], SRCS[2:] -> SRCS[1], ...,
template<class DST, class MID, class ... SRCS> 
class ITransformsA2A<DST, MID, SRCS...>: public ITransformsTo<
    /* BASE */ ITransformsA2A</*DST*/MID, SRCS...>, 
    /* DST */ DST,
    /* SRC */ MID,
    SRCS...>
{
public:
    using ITransformsTo<
    /* BASE */ ITransformsA2A</*DST*/MID, SRCS...>, 
    /* DST */ DST,
    /* SRC */ MID,
    SRCS...>::transform;

};

template<class ...CS>
class ITransforms: public ITransformsA2A<CS...> {
public:
    using ITransformsA2A<CS...>::transform;
    template<class T_DST, class T_SRC>
    auto to(const T_SRC &p) const {
        return transform(p, Tag<T_DST>{});
    }
};


void func(const ITransforms<CsD, CsC, CsB> &t)
{
    auto p_b = Point<CsB>{10};
    auto p_c = Point<CsC>{100 + 1};
    auto p_d = Point<CsD>{1000 + 100 + 1};

    EXPECT_EQ(t.to<Point<CsC>>(p_b), p_c);
    EXPECT_EQ(t.to<Point<CsD>>(p_b), p_d);
    EXPECT_EQ(t.to<Point<CsD>>(p_c), p_d);

}

template<class COMPOSED, class ...CS>
class TransformsHolder: public ITransforms<CS...>
{
public:
    const COMPOSED composed;
};


template<class...> 
class TransformsTo;


template<class BASE, class DST> 
class TransformsTo<BASE, DST>: public BASE
{
public:

};

/// Provides transform() for SRCS->DST, i.e.: SRC->DST SRCS[0]->DST, SRCS[1]->DST, ...
template<class BASE, class DST, class SRC, class ... SRCS> 
class TransformsTo<BASE, DST, SRC, SRCS...>: public TransformsTo<BASE, DST, SRCS...>
{
public:
    using Base = TransformsTo<BASE, DST, SRCS...>;
    Point<DST> transform(const Point<SRC>& p, Tag<Point<DST>> dummy_dst={}) const override
    {
        return BASE::composed.transform(p, Tag<Point<DST>>{});
    }

};


template<class...>
class TransformsA2A;

template<class BASE, class DST>
class TransformsA2A<BASE, DST>: public BASE
{
public:
};

/// Provides transform() for SRCS[1:] -> SRCS[0], SRCS[2:] -> SRCS[1], ...,
template<class BASE, class DST, class MID, class ... SRCS> 
class TransformsA2A<BASE, DST, MID, SRCS...>: public TransformsTo<
    /* BASE */ TransformsA2A<BASE, MID, SRCS...>, 
    /* DST */ DST,
    /* SRC */ MID,
    SRCS...>
{
public:

};

template<class COMPOSED, class ...CS>
class Transforms: public TransformsA2A<TransformsHolder<COMPOSED, CS...>, CS...>
{
public:
    Transforms(const COMPOSED& composed): TransformsHolder<COMPOSED, CS...>{composed} {}
};

template<class COMPOSED>
class Adapter
{
public:
    Adapter(const COMPOSED& composed): composed(composed) {}


    
private:
    const COMPOSED composed;

};


TEST(TransformsTest, CanBePassedToFunctions) {
    Transform<CsB, CsA> t_b_from_a{10};
    Transform<CsC, CsB> t_c_from_b{100};
    Transform<CsD, CsC> t_d_from_c{1000};

    auto t_multi = compose_transforms(t_d_from_c, t_c_from_b, t_b_from_a);
    func(Transforms<CsD, CsC, CsB>{t_multi});
}
#endif

// template<class ...CS>
// class Transforms: public ITransforms<CS...> {
// public:    
// };


#if 0
template<class...>
class ITransforms; // primary

// baza: 0 albo 1 typ -> brak par do wygenerowania
template<>
class ITransforms<> {
public:
    virtual ~ITransforms() = default;
};

template<class LAST>
class ITransforms<LAST> {
public:
    virtual ~ITransforms() = default;
};

// TODO: fix this to be finite recursion
template<class BASE, class DST, class ... SRCS> 
class ITransforms: public ITransforms<SRCS...>, public ITransform<DST, SRCS>... 
{
public:
};



template<class DST, class SRC>
class TransformsToFrom: {
public:
    Point<DST> transform(const Point<SRC>& p) const {
        return Point<DST>{p.x};
    }
};

template<class DST, class ... SRCS> // <A, B, C> means we support any transforms in direction C->B->A
class TransformsTo: public TransformsToFrom<DST, SRCS>... {
public:
};



// This is a wrapper to be used as an interface to functions to avoid making them templates while still allowing passing any composition of transforms. It should be constructible from any composition of transforms and allow to call to<T_DST>(p) for any T_DST supported by the given composition.
template<class ... Cs> // <A, B, C> means we support any transforms in direction C->B->A
class Transforms {
public:
    template<class COMPOSED>
    Transforms(const COMPOSED& composed) {
        // TODO: it should wrap somehow COMPOSED to allow transforms between all given 
    }
};



void func(const Transforms<CsD, CsC, CsB> &t)
{
    auto p_b = Point<CsB>{10};
    auto p_c = Point<CsC>{100 + 1};
    auto p_d = Point<CsD>{1000 + 100 + 1};

    EXPECT_EQ(t.to<Point<CsB>>(p_c), p_b);
    EXPECT_EQ(t.to<Point<CsC>>(p_b), p_c);
    EXPECT_EQ(t.to<Point<CsD>>(p_c), p_d);
}

TEST(TransformsTest, CanBePassedToFunctions) {
    Transform<CsB, CsA> t_b_from_a{10};
    Transform<CsC, CsB> t_c_from_b{100};
    Transform<CsD, CsC> t_d_from_c{1000};

    auto t_multi = compose_transforms(t_d_from_c, t_c_from_b, t_b_from_a);
    func(t_multi);
}

#endif