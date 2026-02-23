#include <gtest/gtest.h>


// Helper matcher for comparing points with tolerance
#include <cmath>
#include <ostream>
#include <gmock/gmock.h>


struct CsA {
};

struct CsB {
};

struct CsC {
};

struct CsD {
};

template<class CS>
struct Point{
    using Cs = CS;
    int x;
    bool operator==(const Point& other) const {
        return x == other.x;
    }



};

template<class CS>
std::ostream& operator<<(std::ostream& os, const Point<CS>& p) {
    os << "Point<" << typeid(CS).name() << ">{x=" << p.x << "}";
    return os;
}

template<class T>
struct Tag{};



template <class CS_DST, class CS_SRC>
class Transform
{
public:
    using CsDst =  CS_DST;
    using CsSrc =  CS_SRC;

    int delta;

    Transform(int d) : delta(d) {}

    Point<CS_DST> transform(const Point<CS_SRC> &in, Tag<Point<CS_DST>> dummy_dst={}) const {
        return Point<CS_DST>{in.x + delta};
    }
};

// given type has transform(T_SRC, Tag<T_DST>) -> T_DST
template<class TRANSFORM, class T_DST, class T_SRC>
concept CanTransform = requires(const TRANSFORM& t, const T_SRC& p, const Tag<T_DST>& dummy_dst) {
    { t.transform(p, dummy_dst) } -> std::same_as<T_DST>;
};

/// TRANSFORM can transform T_SRC into given type
template<class T_DST, class TRANSFORM, class T_SRC>
concept TransformDst =
    std::same_as<
        decltype(std::declval<const TRANSFORM>().transform(std::declval<const T_SRC&>(), Tag<T_DST>{})),
        T_DST>;

/// TRANSFORM can transform given type into T_DST
template<class T_SRC, class TRANSFORM, class T_DST>
concept TransformSrc =
    std::same_as<
        decltype( std::declval<const TRANSFORM>().transform(std::declval<const T_SRC&>(), Tag<T_DST>{})),
        T_DST>;

TEST(Geometry2Tests, SingleTransform) {
    Transform<CsB, CsA> t_b_from_a{10};
    Point<CsA> p_a{1};
    static_assert(TransformDst<Point<CsB>, Transform<CsB, CsA>, Point<CsA>>);
    static_assert(TransformSrc<Point<CsA>, Transform<CsB, CsA>, Point<CsB>>);
    Point<CsB> p_b{10 + 1};
    auto p_b_from_a = t_b_from_a.transform(p_a);
    EXPECT_EQ(p_b_from_a, p_b);
    EXPECT_TRUE(true); // dummy assertion to avoid "no tests" warning

}

// TODO: Transform Concept
// concept IsTransform = requires(const auto& t, const auto& p, const auto& dummy_dst) {


template<class ...>
class Composed;

template<class TRANSFORM>
class Composed<TRANSFORM>: public TRANSFORM
{
public:
    Composed(const TRANSFORM &t) : TRANSFORM{t} {}
};

template <class TRANSFORM, class ...TRANSFORMS>
class Composed<TRANSFORM, TRANSFORMS...>: public TRANSFORM, public Composed<TRANSFORMS...>
{
public:

    Composed(const TRANSFORM &t, const TRANSFORMS &...transforms) : TRANSFORM{t}, Composed<TRANSFORMS...>{transforms...} {}


    // T_SRC --INNER--> Point<TRANSFORM::CsSrc> --TRANSFORM--> Point<TRANSFORM::CsDst>
    using Inner = Composed<TRANSFORMS...>;
    using Inner::transform;
    using TRANSFORM::transform;


    // T_SRC --INNER--> Point<TRANSFORM::CsSrc> --TRANSFORM--> Point<TRANSFORM::CsDst>
    auto transform(TransformSrc<Inner, Point<typename TRANSFORM::CsSrc>> auto p,
                Tag<Point<typename TRANSFORM::CsDst>> dummy_dst = {}) const
    {
        return TRANSFORM::transform(Inner::transform(p, Tag<Point<typename TRANSFORM::CsSrc>>{}), dummy_dst);
    }

    // TRANSFORM::CsSrc --TRANSFORM--> Point<TRANSFORM::CsDst> --INNER--> T_DST
    template<TransformDst<Inner, Point<typename TRANSFORM::CsDst>> T_DST>
    auto transform(const Point<typename TRANSFORM::CsSrc>& p,
                Tag<T_DST> dummy_dst = {}) const
    {
        return Inner::transform(TRANSFORM::transform(p, Tag<Point<typename TRANSFORM::CsDst>>{}), dummy_dst);
    }


    template<class T_DST, class T_SRC>
    auto to(const T_SRC &p) const {
        return transform(p, Tag<T_DST>{});
    }

};

auto compose_transforms(auto transform) {
    return transform;
}

auto compose_transforms(auto transform, auto... others) {
    return Composed<decltype(transform), decltype(others)...>(transform, others...);
}

TEST(ComposedTest, HandlesAllTransformsWhenComposingTwo) {
    Transform<CsB, CsA> t_b_from_a{10};
    Transform<CsC, CsB> t_c_from_b{100};
    Point<CsA> p_a{1};
    Point<CsB> p_b{10 + 1};
    Point<CsC> p_c{100 + 10 + 1};
    
    // Composed<Transform<CsB, CsA>, Transform<CsC, CsB>> t_multi2;
    auto t_multi = compose_transforms(t_c_from_b, t_b_from_a);
    auto p_b_from_a = t_multi.to<Point<CsB>>(p_a);
    auto p_c_from_a = t_multi.to<Point<CsC>>(p_a);
    auto p_c_from_b = t_multi.to<Point<CsC>>(p_b);
    EXPECT_EQ(p_b_from_a, p_b);
    EXPECT_EQ(p_c_from_a, p_c);
    EXPECT_EQ(p_c_from_b, p_c);
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

TEST(ComposedTest, HandlesAllTransformsWhenComposingThree) {
    Transform<CsB, CsA> t_b_from_a{10};
    Transform<CsC, CsB> t_c_from_b{100};
    Transform<CsD, CsC> t_d_from_c{1000};
    Point<CsA> p_a{1};
    Point<CsB> p_b{10 + 1};
    Point<CsC> p_c{100 + 10 + 1};
    Point<CsD> p_d{1000 + 100 + 10 + 1};


    auto t_multi = compose_transforms(t_d_from_c, t_c_from_b, t_b_from_a);

    EXPECT_EQ(p_b, t_multi.to<Point<CsB>>(p_a));
    EXPECT_EQ(p_c, t_multi.to<Point<CsC>>(p_a));
    EXPECT_EQ(p_c, t_multi.to<Point<CsC>>(p_b));
    EXPECT_EQ(p_d, t_multi.to<Point<CsD>>(p_a));
    EXPECT_EQ(p_d, t_multi.to<Point<CsD>>(p_b));
    EXPECT_EQ(p_d, t_multi.to<Point<CsD>>(p_c));    
}

TEST(ComposedTest, HandlesAllTransformsWhenComposingThreeInDifferentOrder) {
    Transform<CsB, CsA> t_b_from_a{10};
    Transform<CsC, CsB> t_c_from_b{100};
    Transform<CsD, CsC> t_d_from_c{1000};
    Point<CsA> p_a{1};
    Point<CsB> p_b{10 + 1};
    Point<CsC> p_c{100 + 10 + 1};
    Point<CsD> p_d{1000 + 100 + 10 + 1};

    // any order but must introduce one new cs at a time (cannot connect two already existing ones)
    auto t_multi = compose_transforms(t_b_from_a, t_d_from_c, t_c_from_b);

    EXPECT_EQ(p_b, t_multi.to<Point<CsB>>(p_a));
    EXPECT_EQ(p_c, t_multi.to<Point<CsC>>(p_a));
    EXPECT_EQ(p_c, t_multi.to<Point<CsC>>(p_b));
    EXPECT_EQ(p_d, t_multi.to<Point<CsD>>(p_a));
    EXPECT_EQ(p_d, t_multi.to<Point<CsD>>(p_b));
    EXPECT_EQ(p_d, t_multi.to<Point<CsD>>(p_c));     
}

template<class TRUE_TRANSFORM, class T_DST, class T_SRC>
T_DST wrapped_transform(const void *transform, const T_SRC &p, Tag<T_DST> dummy_dst={}) {
    const TRUE_TRANSFORM* t = static_cast<const TRUE_TRANSFORM*>(transform);
    return t->transform(p, dummy_dst);
}





///////////////////////////////////

template<class T_DST, class T_SRC>
class SingleTransformHolder
{
protected:
    using FuncPtr = T_DST(*)(const void*, const T_SRC&, Tag<T_DST>);
    const FuncPtr func_ptr;
public:
    SingleTransformHolder(const CanTransform<T_DST, T_SRC> auto &t)
        : func_ptr(&wrapped_transform<std::decay_t<decltype(t)>, T_DST, T_SRC>)
    {
    }

};



/// Provides transform() for SRCS->DST, i.e.: SRCS[0]->DST, SRCS[1]->DST, ...
template<class DST, class ... SRCS> 
class TransformsTo: protected SingleTransformHolder<Point<DST>, Point<SRCS>>...
{
public:
    template<class TRUE_TRANSFORM>
    TransformsTo(const TRUE_TRANSFORM &t): SingleTransformHolder<Point<DST>, Point<SRCS>>{t}... {}

};




template<class...>
class TransformsA2A;

template<class DST>
class TransformsA2A<DST>
{
public:
    template<class TRUE_TRANSFORM>
    TransformsA2A(const TRUE_TRANSFORM &t) {}
};

/// Provides transform() for SRCS[1:]->SRCS[0], SRCS[2:]->SRCS[1], ...,
template<class DST, class MID, class ... SRCS> 
class TransformsA2A<DST, MID, SRCS...>: 
    public TransformsA2A</*DST=*/MID, SRCS...>,
    public TransformsTo</*DST=*/ DST, /*SRC=*/ MID, SRCS...>
{
    using Base1 = TransformsTo</*DST=*/ DST, /*SRC=*/ MID, SRCS...>;
    using Base2 = TransformsA2A</*DST=*/MID, SRCS...>;

public:
    template<class TRUE_TRANSFORM>
    TransformsA2A(const TRUE_TRANSFORM &t): Base2{t}, Base1{t} {}
};


template<class ...CS>
class Transforms: public TransformsA2A<CS...>
{
    using Base = TransformsA2A<CS...>;
public:
    template<class TRUE_TRANSFORM>
    Transforms(const TRUE_TRANSFORM& true_transform_): Base{true_transform_}, true_transform{&true_transform_} {}


    template<class T_DST, class T_SRC>
    T_DST transform(const T_SRC &p, Tag<T_DST> dummy_dst={}) const {
        return SingleTransformHolder<T_DST, T_SRC>::func_ptr(true_transform, p, dummy_dst);
    }


    template<class T_DST, class T_SRC>
    auto to(const T_SRC &p) const {
        return transform(p, Tag<T_DST>{});
    }
private:
    const void *true_transform;
};




void func(const Transforms<CsD, CsC, CsB> &t)
{
    auto p_b = Point<CsB>{10};
    auto p_c = Point<CsC>{100 + 10};
    auto p_d = Point<CsD>{1000 + 100 + 10};

    EXPECT_EQ(t.to<Point<CsC>>(p_b), p_c);
    EXPECT_EQ(t.to<Point<CsD>>(p_b), p_d);
    EXPECT_EQ(t.to<Point<CsD>>(p_c), p_d);

}



TEST(TransformsTest, CanBePassedToFunctions) {
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