#include "compose.hpp"

template <typename C> struct Tag {};

struct G0T0 { int x; };
struct G0T1 { int x; };
struct G0T2 { int x; };
struct G0T3 { int x; };
struct G0T4 { int x; };
struct G0T5 { int x; };
struct G0T6 { int x; };
struct G0T7 { int x; };
struct G0T8 { int x; };
struct G0T9 { int x; };
struct G1T0 { int x; };
struct G1T1 { int x; };
struct G1T2 { int x; };
struct G1T3 { int x; };
struct G1T4 { int x; };
struct G1T5 { int x; };
struct G1T6 { int x; };
struct G1T7 { int x; };
struct G1T8 { int x; };
struct G1T9 { int x; };
struct G2T0 { int x; };
struct G2T1 { int x; };
struct G2T2 { int x; };
struct G2T3 { int x; };
struct G2T4 { int x; };
struct G2T5 { int x; };
struct G2T6 { int x; };
struct G2T7 { int x; };
struct G2T8 { int x; };
struct G2T9 { int x; };
struct G3T0 { int x; };
struct G3T1 { int x; };
struct G3T2 { int x; };
struct G3T3 { int x; };
struct G3T4 { int x; };
struct G3T5 { int x; };
struct G3T6 { int x; };
struct G3T7 { int x; };
struct G3T8 { int x; };
struct G3T9 { int x; };

template <class DST, class SRC>
struct Bi {
    int delta;
    DST operator()(const SRC& p, Tag<DST> = {}) const { return DST{p.x + delta}; }
    SRC operator()(const DST& p, Tag<SRC> = {}) const { return SRC{p.x - delta}; }
    using transformations = transform_list<transform_spec<DST, SRC>, transform_spec<SRC, DST>>;
};

int run() {
    auto t = compose_transforms(
        Bi<G0T1, G0T0>{1},
        Bi<G0T2, G0T0>{2},
        Bi<G0T3, G0T1>{3},
        Bi<G0T4, G0T1>{4},
        Bi<G0T5, G0T2>{5},
        Bi<G0T6, G0T2>{6},
        Bi<G0T7, G0T3>{7},
        Bi<G0T8, G0T3>{8},
        Bi<G0T9, G0T4>{9},
        Bi<G1T1, G1T0>{101},
        Bi<G1T2, G1T0>{102},
        Bi<G1T3, G1T1>{103},
        Bi<G1T4, G1T1>{104},
        Bi<G1T5, G1T2>{105},
        Bi<G1T6, G1T2>{106},
        Bi<G1T7, G1T3>{107},
        Bi<G1T8, G1T3>{108},
        Bi<G1T9, G1T4>{109},
        Bi<G2T1, G2T0>{201},
        Bi<G2T2, G2T0>{202},
        Bi<G2T3, G2T1>{203},
        Bi<G2T4, G2T1>{204},
        Bi<G2T5, G2T2>{205},
        Bi<G2T6, G2T2>{206},
        Bi<G2T7, G2T3>{207},
        Bi<G2T8, G2T3>{208},
        Bi<G2T9, G2T4>{209},
        Bi<G3T1, G3T0>{301},
        Bi<G3T2, G3T0>{302},
        Bi<G3T3, G3T1>{303},
        Bi<G3T4, G3T1>{304},
        Bi<G3T5, G3T2>{305},
        Bi<G3T6, G3T2>{306},
        Bi<G3T7, G3T3>{307},
        Bi<G3T8, G3T3>{308},
        Bi<G3T9, G3T4>{309});
    int sum = 0;
    sum += t.to<G0T1>(G0T0{1}).x;
    sum += t.to<G0T2>(G0T0{1}).x;
    sum += t.to<G0T3>(G0T0{1}).x;
    sum += t.to<G0T4>(G0T0{1}).x;
    sum += t.to<G0T5>(G0T0{1}).x;
    sum += t.to<G0T6>(G0T0{1}).x;
    sum += t.to<G0T7>(G0T0{1}).x;
    sum += t.to<G0T8>(G0T0{1}).x;
    sum += t.to<G0T9>(G0T0{1}).x;
    sum += t.to<G0T0>(G0T1{1}).x;
    sum += t.to<G0T2>(G0T1{1}).x;
    sum += t.to<G0T3>(G0T1{1}).x;
    sum += t.to<G0T4>(G0T1{1}).x;
    sum += t.to<G0T5>(G0T1{1}).x;
    sum += t.to<G0T6>(G0T1{1}).x;
    sum += t.to<G0T7>(G0T1{1}).x;
    sum += t.to<G0T8>(G0T1{1}).x;
    sum += t.to<G0T9>(G0T1{1}).x;
    sum += t.to<G0T0>(G0T2{1}).x;
    sum += t.to<G0T1>(G0T2{1}).x;
    sum += t.to<G0T3>(G0T2{1}).x;
    sum += t.to<G0T4>(G0T2{1}).x;
    sum += t.to<G0T5>(G0T2{1}).x;
    sum += t.to<G0T6>(G0T2{1}).x;
    sum += t.to<G0T7>(G0T2{1}).x;
    sum += t.to<G0T8>(G0T2{1}).x;
    sum += t.to<G0T9>(G0T2{1}).x;
    sum += t.to<G0T0>(G0T3{1}).x;
    sum += t.to<G0T1>(G0T3{1}).x;
    sum += t.to<G0T2>(G0T3{1}).x;
    sum += t.to<G0T4>(G0T3{1}).x;
    sum += t.to<G0T5>(G0T3{1}).x;
    sum += t.to<G0T6>(G0T3{1}).x;
    sum += t.to<G0T7>(G0T3{1}).x;
    sum += t.to<G0T8>(G0T3{1}).x;
    sum += t.to<G0T9>(G0T3{1}).x;
    sum += t.to<G0T0>(G0T4{1}).x;
    sum += t.to<G0T1>(G0T4{1}).x;
    sum += t.to<G0T2>(G0T4{1}).x;
    sum += t.to<G0T3>(G0T4{1}).x;
    sum += t.to<G0T5>(G0T4{1}).x;
    sum += t.to<G0T6>(G0T4{1}).x;
    sum += t.to<G0T7>(G0T4{1}).x;
    sum += t.to<G0T8>(G0T4{1}).x;
    sum += t.to<G0T9>(G0T4{1}).x;
    sum += t.to<G0T0>(G0T5{1}).x;
    sum += t.to<G0T1>(G0T5{1}).x;
    sum += t.to<G0T2>(G0T5{1}).x;
    sum += t.to<G0T3>(G0T5{1}).x;
    sum += t.to<G0T4>(G0T5{1}).x;
    sum += t.to<G0T6>(G0T5{1}).x;
    sum += t.to<G0T7>(G0T5{1}).x;
    sum += t.to<G0T8>(G0T5{1}).x;
    sum += t.to<G0T9>(G0T5{1}).x;
    sum += t.to<G0T0>(G0T6{1}).x;
    sum += t.to<G0T1>(G0T6{1}).x;
    sum += t.to<G0T2>(G0T6{1}).x;
    sum += t.to<G0T3>(G0T6{1}).x;
    sum += t.to<G0T4>(G0T6{1}).x;
    sum += t.to<G0T5>(G0T6{1}).x;
    sum += t.to<G0T7>(G0T6{1}).x;
    sum += t.to<G0T8>(G0T6{1}).x;
    sum += t.to<G0T9>(G0T6{1}).x;
    sum += t.to<G0T0>(G0T7{1}).x;
    sum += t.to<G0T1>(G0T7{1}).x;
    sum += t.to<G0T2>(G0T7{1}).x;
    sum += t.to<G0T3>(G0T7{1}).x;
    sum += t.to<G0T4>(G0T7{1}).x;
    sum += t.to<G0T5>(G0T7{1}).x;
    sum += t.to<G0T6>(G0T7{1}).x;
    sum += t.to<G0T8>(G0T7{1}).x;
    sum += t.to<G0T9>(G0T7{1}).x;
    sum += t.to<G0T0>(G0T8{1}).x;
    sum += t.to<G0T1>(G0T8{1}).x;
    sum += t.to<G0T2>(G0T8{1}).x;
    sum += t.to<G0T3>(G0T8{1}).x;
    sum += t.to<G0T4>(G0T8{1}).x;
    sum += t.to<G0T5>(G0T8{1}).x;
    sum += t.to<G0T6>(G0T8{1}).x;
    sum += t.to<G0T7>(G0T8{1}).x;
    sum += t.to<G0T9>(G0T8{1}).x;
    sum += t.to<G0T0>(G0T9{1}).x;
    sum += t.to<G0T1>(G0T9{1}).x;
    sum += t.to<G0T2>(G0T9{1}).x;
    sum += t.to<G0T3>(G0T9{1}).x;
    sum += t.to<G0T4>(G0T9{1}).x;
    sum += t.to<G0T5>(G0T9{1}).x;
    sum += t.to<G0T6>(G0T9{1}).x;
    sum += t.to<G0T7>(G0T9{1}).x;
    sum += t.to<G0T8>(G0T9{1}).x;
    sum += t.to<G1T1>(G1T0{1}).x;
    sum += t.to<G1T2>(G1T0{1}).x;
    sum += t.to<G1T3>(G1T0{1}).x;
    sum += t.to<G1T4>(G1T0{1}).x;
    sum += t.to<G1T5>(G1T0{1}).x;
    sum += t.to<G1T6>(G1T0{1}).x;
    sum += t.to<G1T7>(G1T0{1}).x;
    sum += t.to<G1T8>(G1T0{1}).x;
    sum += t.to<G1T9>(G1T0{1}).x;
    sum += t.to<G1T0>(G1T1{1}).x;
    sum += t.to<G1T2>(G1T1{1}).x;
    sum += t.to<G1T3>(G1T1{1}).x;
    sum += t.to<G1T4>(G1T1{1}).x;
    sum += t.to<G1T5>(G1T1{1}).x;
    sum += t.to<G1T6>(G1T1{1}).x;
    sum += t.to<G1T7>(G1T1{1}).x;
    sum += t.to<G1T8>(G1T1{1}).x;
    sum += t.to<G1T9>(G1T1{1}).x;
    sum += t.to<G1T0>(G1T2{1}).x;
    sum += t.to<G1T1>(G1T2{1}).x;
    sum += t.to<G1T3>(G1T2{1}).x;
    sum += t.to<G1T4>(G1T2{1}).x;
    sum += t.to<G1T5>(G1T2{1}).x;
    sum += t.to<G1T6>(G1T2{1}).x;
    sum += t.to<G1T7>(G1T2{1}).x;
    sum += t.to<G1T8>(G1T2{1}).x;
    sum += t.to<G1T9>(G1T2{1}).x;
    sum += t.to<G1T0>(G1T3{1}).x;
    sum += t.to<G1T1>(G1T3{1}).x;
    sum += t.to<G1T2>(G1T3{1}).x;
    sum += t.to<G1T4>(G1T3{1}).x;
    sum += t.to<G1T5>(G1T3{1}).x;
    sum += t.to<G1T6>(G1T3{1}).x;
    sum += t.to<G1T7>(G1T3{1}).x;
    sum += t.to<G1T8>(G1T3{1}).x;
    sum += t.to<G1T9>(G1T3{1}).x;
    sum += t.to<G1T0>(G1T4{1}).x;
    sum += t.to<G1T1>(G1T4{1}).x;
    sum += t.to<G1T2>(G1T4{1}).x;
    sum += t.to<G1T3>(G1T4{1}).x;
    sum += t.to<G1T5>(G1T4{1}).x;
    sum += t.to<G1T6>(G1T4{1}).x;
    sum += t.to<G1T7>(G1T4{1}).x;
    sum += t.to<G1T8>(G1T4{1}).x;
    sum += t.to<G1T9>(G1T4{1}).x;
    sum += t.to<G1T0>(G1T5{1}).x;
    sum += t.to<G1T1>(G1T5{1}).x;
    sum += t.to<G1T2>(G1T5{1}).x;
    sum += t.to<G1T3>(G1T5{1}).x;
    sum += t.to<G1T4>(G1T5{1}).x;
    sum += t.to<G1T6>(G1T5{1}).x;
    sum += t.to<G1T7>(G1T5{1}).x;
    sum += t.to<G1T8>(G1T5{1}).x;
    sum += t.to<G1T9>(G1T5{1}).x;
    sum += t.to<G1T0>(G1T6{1}).x;
    sum += t.to<G1T1>(G1T6{1}).x;
    sum += t.to<G1T2>(G1T6{1}).x;
    sum += t.to<G1T3>(G1T6{1}).x;
    sum += t.to<G1T4>(G1T6{1}).x;
    sum += t.to<G1T5>(G1T6{1}).x;
    sum += t.to<G1T7>(G1T6{1}).x;
    sum += t.to<G1T8>(G1T6{1}).x;
    sum += t.to<G1T9>(G1T6{1}).x;
    sum += t.to<G1T0>(G1T7{1}).x;
    sum += t.to<G1T1>(G1T7{1}).x;
    sum += t.to<G1T2>(G1T7{1}).x;
    sum += t.to<G1T3>(G1T7{1}).x;
    sum += t.to<G1T4>(G1T7{1}).x;
    sum += t.to<G1T5>(G1T7{1}).x;
    sum += t.to<G1T6>(G1T7{1}).x;
    sum += t.to<G1T8>(G1T7{1}).x;
    sum += t.to<G1T9>(G1T7{1}).x;
    sum += t.to<G1T0>(G1T8{1}).x;
    sum += t.to<G1T1>(G1T8{1}).x;
    sum += t.to<G1T2>(G1T8{1}).x;
    sum += t.to<G1T3>(G1T8{1}).x;
    sum += t.to<G1T4>(G1T8{1}).x;
    sum += t.to<G1T5>(G1T8{1}).x;
    sum += t.to<G1T6>(G1T8{1}).x;
    sum += t.to<G1T7>(G1T8{1}).x;
    sum += t.to<G1T9>(G1T8{1}).x;
    sum += t.to<G1T0>(G1T9{1}).x;
    sum += t.to<G1T1>(G1T9{1}).x;
    sum += t.to<G1T2>(G1T9{1}).x;
    sum += t.to<G1T3>(G1T9{1}).x;
    sum += t.to<G1T4>(G1T9{1}).x;
    sum += t.to<G1T5>(G1T9{1}).x;
    sum += t.to<G1T6>(G1T9{1}).x;
    sum += t.to<G1T7>(G1T9{1}).x;
    sum += t.to<G1T8>(G1T9{1}).x;
    sum += t.to<G2T1>(G2T0{1}).x;
    sum += t.to<G2T2>(G2T0{1}).x;
    sum += t.to<G2T3>(G2T0{1}).x;
    sum += t.to<G2T4>(G2T0{1}).x;
    sum += t.to<G2T5>(G2T0{1}).x;
    sum += t.to<G2T6>(G2T0{1}).x;
    sum += t.to<G2T7>(G2T0{1}).x;
    sum += t.to<G2T8>(G2T0{1}).x;
    sum += t.to<G2T9>(G2T0{1}).x;
    sum += t.to<G2T0>(G2T1{1}).x;
    sum += t.to<G2T2>(G2T1{1}).x;
    sum += t.to<G2T3>(G2T1{1}).x;
    sum += t.to<G2T4>(G2T1{1}).x;
    sum += t.to<G2T5>(G2T1{1}).x;
    sum += t.to<G2T6>(G2T1{1}).x;
    sum += t.to<G2T7>(G2T1{1}).x;
    sum += t.to<G2T8>(G2T1{1}).x;
    sum += t.to<G2T9>(G2T1{1}).x;
    sum += t.to<G2T0>(G2T2{1}).x;
    sum += t.to<G2T1>(G2T2{1}).x;
    sum += t.to<G2T3>(G2T2{1}).x;
    sum += t.to<G2T4>(G2T2{1}).x;
    sum += t.to<G2T5>(G2T2{1}).x;
    sum += t.to<G2T6>(G2T2{1}).x;
    sum += t.to<G2T7>(G2T2{1}).x;
    sum += t.to<G2T8>(G2T2{1}).x;
    sum += t.to<G2T9>(G2T2{1}).x;
    sum += t.to<G2T0>(G2T3{1}).x;
    sum += t.to<G2T1>(G2T3{1}).x;
    sum += t.to<G2T2>(G2T3{1}).x;
    sum += t.to<G2T4>(G2T3{1}).x;
    sum += t.to<G2T5>(G2T3{1}).x;
    sum += t.to<G2T6>(G2T3{1}).x;
    sum += t.to<G2T7>(G2T3{1}).x;
    sum += t.to<G2T8>(G2T3{1}).x;
    sum += t.to<G2T9>(G2T3{1}).x;
    sum += t.to<G2T0>(G2T4{1}).x;
    sum += t.to<G2T1>(G2T4{1}).x;
    sum += t.to<G2T2>(G2T4{1}).x;
    sum += t.to<G2T3>(G2T4{1}).x;
    sum += t.to<G2T5>(G2T4{1}).x;
    sum += t.to<G2T6>(G2T4{1}).x;
    sum += t.to<G2T7>(G2T4{1}).x;
    sum += t.to<G2T8>(G2T4{1}).x;
    sum += t.to<G2T9>(G2T4{1}).x;
    sum += t.to<G2T0>(G2T5{1}).x;
    sum += t.to<G2T1>(G2T5{1}).x;
    sum += t.to<G2T2>(G2T5{1}).x;
    sum += t.to<G2T3>(G2T5{1}).x;
    sum += t.to<G2T4>(G2T5{1}).x;
    sum += t.to<G2T6>(G2T5{1}).x;
    sum += t.to<G2T7>(G2T5{1}).x;
    sum += t.to<G2T8>(G2T5{1}).x;
    sum += t.to<G2T9>(G2T5{1}).x;
    sum += t.to<G2T0>(G2T6{1}).x;
    sum += t.to<G2T1>(G2T6{1}).x;
    sum += t.to<G2T2>(G2T6{1}).x;
    sum += t.to<G2T3>(G2T6{1}).x;
    sum += t.to<G2T4>(G2T6{1}).x;
    sum += t.to<G2T5>(G2T6{1}).x;
    sum += t.to<G2T7>(G2T6{1}).x;
    sum += t.to<G2T8>(G2T6{1}).x;
    sum += t.to<G2T9>(G2T6{1}).x;
    sum += t.to<G2T0>(G2T7{1}).x;
    sum += t.to<G2T1>(G2T7{1}).x;
    sum += t.to<G2T2>(G2T7{1}).x;
    sum += t.to<G2T3>(G2T7{1}).x;
    sum += t.to<G2T4>(G2T7{1}).x;
    sum += t.to<G2T5>(G2T7{1}).x;
    sum += t.to<G2T6>(G2T7{1}).x;
    sum += t.to<G2T8>(G2T7{1}).x;
    sum += t.to<G2T9>(G2T7{1}).x;
    sum += t.to<G2T0>(G2T8{1}).x;
    sum += t.to<G2T1>(G2T8{1}).x;
    sum += t.to<G2T2>(G2T8{1}).x;
    sum += t.to<G2T3>(G2T8{1}).x;
    sum += t.to<G2T4>(G2T8{1}).x;
    sum += t.to<G2T5>(G2T8{1}).x;
    sum += t.to<G2T6>(G2T8{1}).x;
    sum += t.to<G2T7>(G2T8{1}).x;
    sum += t.to<G2T9>(G2T8{1}).x;
    sum += t.to<G2T0>(G2T9{1}).x;
    sum += t.to<G2T1>(G2T9{1}).x;
    sum += t.to<G2T2>(G2T9{1}).x;
    sum += t.to<G2T3>(G2T9{1}).x;
    sum += t.to<G2T4>(G2T9{1}).x;
    sum += t.to<G2T5>(G2T9{1}).x;
    sum += t.to<G2T6>(G2T9{1}).x;
    sum += t.to<G2T7>(G2T9{1}).x;
    sum += t.to<G2T8>(G2T9{1}).x;
    sum += t.to<G3T1>(G3T0{1}).x;
    sum += t.to<G3T2>(G3T0{1}).x;
    sum += t.to<G3T3>(G3T0{1}).x;
    sum += t.to<G3T4>(G3T0{1}).x;
    sum += t.to<G3T5>(G3T0{1}).x;
    sum += t.to<G3T6>(G3T0{1}).x;
    sum += t.to<G3T7>(G3T0{1}).x;
    sum += t.to<G3T8>(G3T0{1}).x;
    sum += t.to<G3T9>(G3T0{1}).x;
    sum += t.to<G3T0>(G3T1{1}).x;
    sum += t.to<G3T2>(G3T1{1}).x;
    sum += t.to<G3T3>(G3T1{1}).x;
    sum += t.to<G3T4>(G3T1{1}).x;
    sum += t.to<G3T5>(G3T1{1}).x;
    sum += t.to<G3T6>(G3T1{1}).x;
    sum += t.to<G3T7>(G3T1{1}).x;
    sum += t.to<G3T8>(G3T1{1}).x;
    sum += t.to<G3T9>(G3T1{1}).x;
    sum += t.to<G3T0>(G3T2{1}).x;
    sum += t.to<G3T1>(G3T2{1}).x;
    sum += t.to<G3T3>(G3T2{1}).x;
    sum += t.to<G3T4>(G3T2{1}).x;
    sum += t.to<G3T5>(G3T2{1}).x;
    sum += t.to<G3T6>(G3T2{1}).x;
    sum += t.to<G3T7>(G3T2{1}).x;
    sum += t.to<G3T8>(G3T2{1}).x;
    sum += t.to<G3T9>(G3T2{1}).x;
    sum += t.to<G3T0>(G3T3{1}).x;
    sum += t.to<G3T1>(G3T3{1}).x;
    sum += t.to<G3T2>(G3T3{1}).x;
    sum += t.to<G3T4>(G3T3{1}).x;
    sum += t.to<G3T5>(G3T3{1}).x;
    sum += t.to<G3T6>(G3T3{1}).x;
    sum += t.to<G3T7>(G3T3{1}).x;
    sum += t.to<G3T8>(G3T3{1}).x;
    sum += t.to<G3T9>(G3T3{1}).x;
    sum += t.to<G3T0>(G3T4{1}).x;
    sum += t.to<G3T1>(G3T4{1}).x;
    sum += t.to<G3T2>(G3T4{1}).x;
    sum += t.to<G3T3>(G3T4{1}).x;
    sum += t.to<G3T5>(G3T4{1}).x;
    sum += t.to<G3T6>(G3T4{1}).x;
    sum += t.to<G3T7>(G3T4{1}).x;
    sum += t.to<G3T8>(G3T4{1}).x;
    sum += t.to<G3T9>(G3T4{1}).x;
    sum += t.to<G3T0>(G3T5{1}).x;
    sum += t.to<G3T1>(G3T5{1}).x;
    sum += t.to<G3T2>(G3T5{1}).x;
    sum += t.to<G3T3>(G3T5{1}).x;
    sum += t.to<G3T4>(G3T5{1}).x;
    sum += t.to<G3T6>(G3T5{1}).x;
    sum += t.to<G3T7>(G3T5{1}).x;
    sum += t.to<G3T8>(G3T5{1}).x;
    sum += t.to<G3T9>(G3T5{1}).x;
    sum += t.to<G3T0>(G3T6{1}).x;
    sum += t.to<G3T1>(G3T6{1}).x;
    sum += t.to<G3T2>(G3T6{1}).x;
    sum += t.to<G3T3>(G3T6{1}).x;
    sum += t.to<G3T4>(G3T6{1}).x;
    sum += t.to<G3T5>(G3T6{1}).x;
    sum += t.to<G3T7>(G3T6{1}).x;
    sum += t.to<G3T8>(G3T6{1}).x;
    sum += t.to<G3T9>(G3T6{1}).x;
    sum += t.to<G3T0>(G3T7{1}).x;
    sum += t.to<G3T1>(G3T7{1}).x;
    sum += t.to<G3T2>(G3T7{1}).x;
    sum += t.to<G3T3>(G3T7{1}).x;
    sum += t.to<G3T4>(G3T7{1}).x;
    sum += t.to<G3T5>(G3T7{1}).x;
    sum += t.to<G3T6>(G3T7{1}).x;
    sum += t.to<G3T8>(G3T7{1}).x;
    sum += t.to<G3T9>(G3T7{1}).x;
    sum += t.to<G3T0>(G3T8{1}).x;
    sum += t.to<G3T1>(G3T8{1}).x;
    sum += t.to<G3T2>(G3T8{1}).x;
    sum += t.to<G3T3>(G3T8{1}).x;
    sum += t.to<G3T4>(G3T8{1}).x;
    sum += t.to<G3T5>(G3T8{1}).x;
    sum += t.to<G3T6>(G3T8{1}).x;
    sum += t.to<G3T7>(G3T8{1}).x;
    sum += t.to<G3T9>(G3T8{1}).x;
    sum += t.to<G3T0>(G3T9{1}).x;
    sum += t.to<G3T1>(G3T9{1}).x;
    sum += t.to<G3T2>(G3T9{1}).x;
    sum += t.to<G3T3>(G3T9{1}).x;
    sum += t.to<G3T4>(G3T9{1}).x;
    sum += t.to<G3T5>(G3T9{1}).x;
    sum += t.to<G3T6>(G3T9{1}).x;
    sum += t.to<G3T7>(G3T9{1}).x;
    sum += t.to<G3T8>(G3T9{1}).x;
    return sum;
}

int main() { return run() != 0; }
