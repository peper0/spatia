#include <concepts>
#include <utility>

template<class T>
struct Tag {};


struct A {};
struct B {};
struct C {};
struct D {};


class TransformABC // C->A, C->B
{
public:
    A transform(const C&, Tag<A> = {}) const
    {
        return {};
    }

    B transform(const C&, Tag<B> = {}) const
    {
        return {};
    }
};


class TransformCD // D->C
{
public:
    C transform(const D&, Tag<C> = {}) const
    {
        return {};
    }
};


template<class Outer, class Inner>
class ComposedTransform
{
public:
    constexpr ComposedTransform(Outer outer, Inner inner)
        : outer_(std::forward<Outer>(outer)),
          inner_(std::forward<Inner>(inner))
    {
    }

    template<class Input, class Output>
        requires requires(
            const Outer& outer,
            const Inner& inner,
            const Input& input)
    {
        outer.transform(
            inner.transform(input),
            Tag<Output>{}
        );
    }
    constexpr decltype(auto) transform(
        const Input& input,
        Tag<Output> = {}
    ) const
        noexcept(noexcept(
            outer_.transform(
                inner_.transform(input),
                Tag<Output>{}
            )
        ))
    {
        return outer_.transform(
            inner_.transform(input),
            Tag<Output>{}
        );
    }

private:
    Outer outer_;
    Inner inner_;
};


template<class Outer, class Inner>
constexpr auto compose(Outer&& outer, Inner&& inner)
{
    return ComposedTransform<Outer, Inner>{
        std::forward<Outer>(outer),
        std::forward<Inner>(inner)
    };
}


int main()
{
    TransformABC t_abc;
    TransformCD  t_cd;

    auto new_transform = compose(t_abc, t_cd);

    auto a = new_transform.transform(D{}, Tag<A>{});
    auto b = new_transform.transform(D{}, Tag<B>{});

    static_assert(std::same_as<decltype(a), A>);
    static_assert(std::same_as<decltype(b), B>);
}