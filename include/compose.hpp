#pragma once

#include "common.hpp"

template <class T, class Tuple>
struct tuple_contains;

template <class T, class... Ts>
struct tuple_contains<T, std::tuple<Ts...>>
    : std::bool_constant<(std::is_same_v<T, Ts> || ...)> {};

template <class T, class Tuple>
inline constexpr bool tuple_contains_v =
    tuple_contains<T, std::remove_cv_t<std::remove_reference_t<Tuple>>>::value;

template<class ...>
class ComposedHelper;

template<class TRANSFORM>
class ComposedHelper<TRANSFORM>: public TRANSFORM
{
public:
    ComposedHelper(const TRANSFORM &t) : TRANSFORM{t} {}

    template<class T>
    static auto transformable_from(T p) { return std::tuple<T>{}; }

    static auto transformable_from(const Point<typename TRANSFORM::CsSrc>& p) {
        return std::tuple<Point<typename TRANSFORM::CsDst>, Point<typename TRANSFORM::CsSrc>>{};
    }

};

/// ComposedHelper<A2B, OTHER_TRANSFORMS>
/// * derives indirectly from all transformations
/// * derives from ComposedHelper<OTHER_TRANSFORMS>
/// * adds transform(A, X) = ComposedHelper<OTHER_TRANSFORMS>::transform(A2B::transform(A, B), X) if OTHER_TRANSFORMS can transform B into something
/// * or transform(X, B) = A2B::transform(ComposedHelper<OTHER_TRANSFORMS>::transform(X, B), A) if OTHER_TRANSFORMS can transform something into A
template <class TRANSFORM, class ...TRANSFORMS>
class ComposedHelper<TRANSFORM, TRANSFORMS...>: public TRANSFORM, public ComposedHelper<TRANSFORMS...>
{
public:

    ComposedHelper(const TRANSFORM &t, const TRANSFORMS &...transforms) : TRANSFORM{t}, ComposedHelper<TRANSFORMS...>{transforms...} {}

    using Inner = ComposedHelper<TRANSFORMS...>;
    using Inner::transform;
    using TRANSFORM::transform;


    static auto transformable_from(const Point<typename TRANSFORM::CsSrc>& p) {
        return std::tuple_cat(
            Inner::transformable_from(p), // inherited from Inner
            std::tuple<Point<typename TRANSFORM::CsDst>>{},  // direct from TRANSFORM
            Inner::transformable_from(Point<typename TRANSFORM::CsDst>{})// CsSrc->TRANSFORM->CsDst->Inner->...
            );
    }


    // from any p non-CsSrc if Inner::transformable_from result contains CsSrc
    static auto transformable_from(const auto& p)// requires (!std::same_as<std::remove_cvref_t<decltype(p)>, Point<typename TRANSFORM::CsSrc>>)
    {
        auto inner_result = Inner::transformable_from(p);
        if constexpr (tuple_contains_v<Point<typename TRANSFORM::CsSrc>, decltype(inner_result)>) {
            return std::tuple_cat(
                std::tuple<Point<typename TRANSFORM::CsDst>>{},  // direct from TRANSFORM
                inner_result
            );
        }
        else {
            return inner_result;
        }
    }



    // TRANSFORM::CsSrc --TRANSFORM--> TRANSFORM::CsDst --INNER--> T_DST
    template<TransformDst<Inner, Point<typename TRANSFORM::CsDst>> T_DST>
    auto transform(const Point<typename TRANSFORM::CsSrc>& p,
                Tag<T_DST> dummy_dst = {}) const
                requires (!std::same_as<std::remove_cvref_t<decltype(p)>, T_DST>)

    {
        return Inner::transform(TRANSFORM::transform(p, Tag<Point<typename TRANSFORM::CsDst>>{}), dummy_dst);
    }

    // T_SRC --INNER--> TRANSFORM::CsSrc --TRANSFORM--> TRANSFORM::CsDst
    template<TransformSrc<Inner, Point<typename TRANSFORM::CsSrc>> T_SRC>
    auto transform(T_SRC p,
                Tag<Point<typename TRANSFORM::CsDst>> dummy_dst = {}) const
                requires (!std::same_as<std::remove_cvref_t<decltype(p)>, Point<typename TRANSFORM::CsDst>>)

    {
        return TRANSFORM::transform(Inner::transform(p, Tag<Point<typename TRANSFORM::CsSrc>>{}), dummy_dst);
    }




};

template <class ...TRANSFORMS>
class Composed: public ComposedHelper<TRANSFORMS...>
{
    using Base = ComposedHelper<TRANSFORMS...>;
public:
    Composed(const TRANSFORMS &...transforms) : Base{transforms...} {}

    using Base::transform;
    
    // Identity transform for completeness
    template<class T>
    T transform(const T &p, Tag<T> dummy_dst = {}) const {
        return p;
    }

    
    template<class T_DST, class T_SRC>
    auto to(const T_SRC &p) const {
        static_assert(tuple_contains_v<T_DST, decltype(Base::transformable_from(p))>, "Cannot transform to the specified destination type");
        return transform(p, Tag<T_DST>{});
    }

};


auto compose_transforms(auto transform, auto... others) {
    return Composed<decltype(transform), decltype(others)...>(transform, others...);
}
