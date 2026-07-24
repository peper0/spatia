#include <gtest/gtest.h>

#include "geometry2.h"

// Helper matcher for comparing points with tolerance
#include "testing.h"

template <class T, int N>
struct CartesianPoint {
};

template <class T>
struct CartesianPoint<T, 2> {
    using This = CartesianPoint<T, 2>;
    T x;
    T y;
    T &operator[](int i) {
        if (i == 0)
            return x;
        else if (i == 1)
            return y;
        else
            throw std::out_of_range("Index out of range");
    }
    const T &operator[](int i) const {
        if (i == 0)
            return x;
        else if (i == 1)
            return y;
        else
            throw std::out_of_range("Index out of range");
    }
};

template <class T>
struct CartesianPoint<T, 3> {
    using This = CartesianPoint<T, 3>;
    T x;
    T y;
    T z;
    T &operator[](int i) {
        if (i == 0)
            return x;
        else if (i == 1)
            return y;
        else if (i == 2)
            return z;
        else
            throw std::out_of_range("Index out of range");
    }

    static This zero() {
        return This{0, 0, 0};
    }
    static This nan() {
        return This{std::numeric_limits<T>::quiet_NaN(),
                    std::numeric_limits<T>::quiet_NaN(),
                    std::numeric_limits<T>::quiet_NaN()};
    }
    static This inf() {
        return This{std::numeric_limits<T>::infinity(),
                    std::numeric_limits<T>::infinity(),
                    std::numeric_limits<T>::infinity()};
    }
};

struct CsA {
    static constexpr int NDIM = 2;
    using Point = CartesianPoint<double, NDIM>;
};

struct CsB {
    static constexpr int NDIM = 2;
    using Point = CartesianPoint<double, NDIM>;
};

struct CsC {
    static constexpr int NDIM = 2;
    using Point = CartesianPoint<double, NDIM>;
};

struct CsD {
    static constexpr int NDIM = 2;
    using Point = CartesianPoint<double, NDIM>;
};


template <class CS>
struct Point: public CS::Point
{

};



template <class CS_DST, class CS_SRC, int N = CS_DST::NDIM>
class Translation {};

template <class CS_DST, class CS_SRC>
class Translation<CS_DST, CS_SRC, 2> 
{
    public:
    using CsDst = CS_DST;
    using CsSrc = CS_SRC;
    double dx;
    double dy;
    auto apply(const Point<CS_SRC> &in, const Point<CS_DST> *dummy_dst=nullptr) const {
        return Point<CS_DST>{in.x + dx, in.y + dy};
    }
};

template<class DST_POINT, class TRANSFORM, class SRC_POINT>
concept TransformableTo =
    std::same_as<
        decltype( std::declval<const TRANSFORM>().apply(
            std::declval<const SRC_POINT&>(),
            static_cast<const DST_POINT*>(nullptr))
        ),
        DST_POINT>;

template<class SRC_POINT, class TRANSFORM, class DST_POINT>
concept TransformableFrom =
    std::same_as<
        decltype( std::declval<const TRANSFORM>().apply(
            std::declval<const SRC_POINT&>(),
            static_cast<const DST_POINT*>(nullptr))
        ),
        DST_POINT>;
        


template <class TRANS, class INNER>
class Compose: public TRANS, public INNER
{
public:
//     template<class POINT_DST>
//    auto apply(const Point<typename TRANS::CsSrc> &p, const POINT_DST *dummy_dst=nullptr) const {
//         return INNER::apply(TRANS::apply(p), dummy_dst);
//     }
    // POINT_SRC --INNER--> Point<TRANS::CsSrc> --TRANS--> Point<TRANS::CsDst>
    using INNER::apply;
    using TRANS::apply;
    // template<class POINT_SRC, 
    //     std::enable_if_t<
    //         std::is_same_v<
    //             std::invoke_result_t<decltype(std::declval<const INNER>().apply(
    //                 std::declval<const POINT_SRC&>(),
    //                 static_cast<const Point<typename TRANS::CsSrc>*>(nullptr))), const INNER*, const POINT_SRC&, const Point<typename TRANS::CsSrc>*>, 
    //             Point<typename TRANS::CsSrc>
    //         >, int> = 0>
    // auto apply(const POINT_SRC &p, const Point<typename TRANS::CsDst> *dummy_dst=nullptr) const {
    //     return TRANS::apply(INNER::apply(p), dummy_dst);
    // }

    // POINT_SRC --INNER--> Point<TRANS::CsSrc> --TRANS--> Point<TRANS::CsDst>
    template<TransformableTo<Point<typename TRANS::CsDst>, INNER> POINT_SRC>
    auto apply(const POINT_SRC& p,
                const Point<typename TRANS::CsDst>* dummy_dst = nullptr) const
    {
        return TRANS::apply(INNER::apply(p), dummy_dst);
    }

    // TRANS::CsSrc --TRANS--> Point<TRANS::CsDst> --INNER--> POINT_DST
    template<TransformableFrom<Point<typename TRANS::CsSrc>, INNER> POINT_DST>
    auto apply(const Point<typename TRANS::CsSrc>& p,
                const POINT_DST* dummy_dst = nullptr) const
    {
        return INNER::apply(TRANS::apply(p), dummy_dst);
    }


    template<class POINT_DST, class POINT_SRC>
    auto to(const POINT_SRC &p) const {
        return apply(p, (POINT_DST*)nullptr);
    }

};



// Example test case
TEST(Geometry2Tests, SingleTransform) {
    Point<CsA> p_a{1.0, 2.0};
    using TransBFromA = Translation<CsB, CsA>;
    TransBFromA t {1.0, 2.0};
    auto p_b = t.apply(p_a);
    EXPECT_THAT(p_b, PointNearTo(Point<CsB>{2.0, 4.0}, 1e-6));

}



// Example test case
TEST(Geometry2Tests, AddFunction) {
    Point<CsA> p{1.0, 2.0};
    using TransBFromA = Translation<CsB, CsA>;
    using TransCFromB = Translation<CsC, CsB>;
    using TransDFromC = Translation<CsD, CsC>;
    // Compose<View3FromView2, Compose<View2FromView, ViewFromPicture>> compose;
    Compose<TransBFromA, Compose<TransDFromC, TransCFromB>> compose;
    TransBFromA &t1 = compose;
    TransCFromB &t2 = compose;
    TransDFromC &t3 = compose;

    t1.dx = 1.0;
    t1.dy = 2.0;
    t2.dx = 10.0;
    t2.dy = 20.0;
    // auto p2=t1.apply(p1);
    auto p1=compose.to<Point<CsB>>(p);
    auto p2_from_1=compose.to<Point<CsC>>(p1);
    auto p3_from_1=compose.to<Point<CsC>>(p1);
    auto p1_from_2 = compose.to<Point<CsA>>(p2_from_1);
    auto p3_from_2=compose.to<Point<CsD>>(p2_from_1);
    using Aaa = std::invoke_result_t<decltype(&TransBFromA::apply), const TransBFromA&, const Point<CsA>&, const Point<CsB>*>;
    static_assert(std::is_same_v<   std::invoke_result_t<decltype(&TransBFromA::apply), const TransBFromA&, const Point<CsA>&, const Point<CsB>*>, Point<CsB>>);  

}