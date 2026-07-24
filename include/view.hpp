#pragma once

#include "common.hpp"

namespace ViewDetail {

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

} // namespace ViewDetail


template<class ...CS>
class TransformsView: public ViewDetail::TransformsA2A<CS...>
{
    using Base = ViewDetail::TransformsA2A<CS...>;
public:
    template<class TRUE_TRANSFORM>
    TransformsView(const TRUE_TRANSFORM& true_transform_): Base{true_transform_}, true_transform{&true_transform_} {}


    template<class T_DST, class T_SRC>
    T_DST transform(const T_SRC &p, Tag<T_DST> dummy_dst={}) const {
        return ViewDetail::SingleTransformHolder<T_DST, T_SRC>::func_ptr(true_transform, p, dummy_dst);
    }


    template<class T_DST, class T_SRC>
    auto to(const T_SRC &p) const {
        return transform(p, Tag<T_DST>{});
    }
private:
    const void *true_transform;
};


