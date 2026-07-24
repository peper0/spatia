#pragma once

#include <ostream>
#include <concepts>
#include <type_traits>
#include <typeinfo>
#include <utility>
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


// An examplary transform
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
template<class T_DST2, class TRANSFORM, class T_SRC>
concept TransformDst =
    std::same_as<
        decltype(std::declval<const TRANSFORM>().transform(std::declval<const T_SRC&>(), Tag<T_DST2>{})),
        T_DST2>;

/// TRANSFORM can transform given type into T_DST
template<class T_SRC, class TRANSFORM, class T_DST>
concept TransformSrc =
    std::same_as<
        decltype( std::declval<const TRANSFORM>().transform(std::declval<const T_SRC&>(), Tag<T_DST>{})),
        T_DST>;