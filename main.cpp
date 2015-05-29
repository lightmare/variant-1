#include "versatile/versatile.hpp"

#include <string>
#include <array>
#include <utility>
#include <tuple>
#include <functional>
#include <sstream>
#if defined(DEBUG) || defined(_DEBUG)
#include <iostream>
#include <iomanip>
#endif

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdlib>

#ifndef COLS
#define COLS 5
#endif

#ifndef ROWS
#define ROWS COLS
#endif

namespace
{

struct introspector
{

    template< typename ...types >
    std::string
    operator () (types...) const
    {
        return __PRETTY_FUNCTION__;
    }

};

template< std::size_t I >
struct T
{

};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-braces"

struct visitor
{

    template< std::size_t ...I >
    constexpr
    std::array< std::size_t, sizeof...(I) >
    operator () (T< I >...) const noexcept
    {
        return {I...};
    }

};

template< std::size_t ...M, std::size_t ...N >
constexpr
bool
invoke(std::index_sequence< M... >, std::index_sequence< N... >) noexcept
{
    using versatile::apply_visitor;
    return (std::array< std::size_t, sizeof...(N) >{(N % sizeof...(M))...} == apply_visitor(visitor{}, versatile::variant< T< M >... >{T< (N % sizeof...(M)) >{}}...));
}

#pragma clang diagnostic pop

template< std::size_t M, std::size_t N = M >
constexpr
bool
test() noexcept
{
    return invoke(std::make_index_sequence< M >{}, std::make_index_sequence< N >{});
}

}

namespace
{

struct visitor0
{

    using R = std::tuple< bool, bool, bool >;

    template< typename type >
    R
    operator () (type &&) const
    {
        return std::make_tuple(true, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value);
    }

    template< typename type >
    R
    operator () (type &&)
    {
        return std::make_tuple(false, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value);
    }

};

struct visitor1
{

    using R = std::tuple< bool, bool, bool, bool >;

    template< typename type >
    R
    operator () (type &&) const &
    {
        return std::make_tuple(true, true, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value);
    }

    template< typename type >
    R
    operator () (type &&) &
    {
        return std::make_tuple(false, true, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value);
    }

    template< typename type >
    R
    operator () (type &&) &&
    {
        return std::make_tuple(false, false, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value);
    }

};

struct visitor2
{
    template< typename ...types >
    auto
    operator () (types &&... _values) const
    {
        return std::tuple_cat(std::make_tuple(true), std::make_tuple(std::tie(typeid(std::forward< types >(_values))), std::is_const< std::remove_reference_t< types > >::value, std::is_lvalue_reference< types >::value)...);
    }

    template< typename ...types >
    auto
    operator () (types &&... _values)
    {
        return std::tuple_cat(std::make_tuple(false), std::make_tuple(std::tie(typeid(std::forward< types >(_values))), std::is_const< std::remove_reference_t< types > >::value, std::is_lvalue_reference< types >::value)...);
    }

};

struct visitor3
{

    using R = std::tuple< bool, bool, bool, bool >;

    template< typename type >
    R
    operator () (type &&) const &
    {
        return std::make_tuple(true, true, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value);
    }

    template< typename type >
    R
    operator () (type &&) &
    {
        return std::make_tuple(false, true, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value);
    }

    template< typename type >
    R
    operator () (type &&) const &&
    {
        return std::make_tuple(true, false, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value);
    }

    template< typename type >
    R
    operator () (type &&) &&
    {
        return std::make_tuple(false, false, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value);
    }

};

}

int
main()
{
    {
        using namespace versatile;
        {
            using V = versatile< int, double >;
            V v{1};
            assert(v.active< int >());
            assert(static_cast< int >(v) == 1);
            V w{2.0};
            assert(w.active< double >());
            assert(static_cast< double >(w) == 2.0);
            //static_assert(std::is_trivially_copy_constructible< V >{}, "versatile is not trivially copy constructible");
            V u{w};
            assert(u.active< double >());
            assert(static_cast< double >(u) == 2.0);
            static_assert(std::is_trivially_copy_assignable< V >{}, "versatile is not trivially copy assignable");
            u = v;
            assert(u.active< int >());
            assert(static_cast< int >(u) == 1);
        }
        {
            using V = variant<>;
            static_assert(V::width == 0, "V::width != 0");
            V v;
            assert(v.which() == 0);
        }
        {
            using V = variant< int >;
            static_assert(V::width == 1, "V::width != 1");
            V v;
            assert((1 == index< V, int >));
            assert(v.index< int >() == 1);
            assert(v.which() == 1);
            assert(v.active< int >());
            assert(static_cast< int >(v) == int{});
            V w(222);
            assert(static_cast< int >(w) == 222);
            v = w;
            assert(static_cast< int >(v) == 222);
            assert(v == w);
            assert(!(v < w));
            assert(!(w < v));
            ++static_cast< int & >(v);
            assert(w < v);
            assert(!(v < w));
            assert(!(v == w));
            assert(static_cast< int >(w) == 222);
            assert(static_cast< int >(v) == 223);
            v.swap(w);
            assert(static_cast< int >(w) == 223);
            assert(static_cast< int >(v) == 222);
            V u = w;
            assert(static_cast< int >(u) == 223);
            assert(w.index< int >() == 1);
            assert(w.which() == 1);
            assert(u.index< int >() == 1);
            assert(u.which() == 1);
        }
        {
            using V = variant< int, float, double, long double >;
            assert((V{}.which() == index< V, int >));
            assert((V{0}.which() == index< V, int >));
            assert((V{1.0f}.which() == index< V, float >));
            assert((V{2.0}.which() == index< V, double >));
            assert((V{3.0L}.which() == index< V, long double >));
            V i;
            assert(i.active< int >());
            V j = 1;
            assert(j.active< int >());
            V f = 1.0f;
            assert(f.active< float >());
            V d = 2.0;
            assert(d.active< double >());
            V l = 3.0L;
            assert(l.active< long double >());
            i = l;
            assert(i.active< long double >());
            l = d;
            assert(l.active< double >());
            d = f;
            assert(d.active< float >());
            f = j;
            assert(f.active< int >());
            d.swap(j);
            assert(d.active< int >());
            assert(j.active< float >());
        }
        {
            struct A { A() = delete; };
            struct B {};
            using V =  variant< A, B >;
            V v;
            assert(v.active< B >());
        }
        {
            struct A {};
            struct B;
            using V = variant< A, B >;
            struct B {};
            V v;
            assert(v.active< A >());
            v = B{};
            assert(v.active< B >());
        }
        {
            struct A {};
            struct B;
            using V = variant< A, recursive_wrapper< B > >;
            V v;
            assert(v.active< A >());
            struct B {};
            v = B{};
            assert(v.active< B >());
        }
        {
            struct R;
            struct A {};
            using V = variant< A, recursive_wrapper< R > >;
            V v;
            assert(v.active< A >());
            struct R { V v; };
            v = R{};
            assert(v.active< R >());
        }
        {
            using V = variant< int, double >;
            assert(!(V{1} == V{1.0}));
            assert(V{} == V{});
            assert(V{} == int{});
            assert(int{} == V{});
            assert(V{1} == 1);
            assert(1 == V{1});
            assert(!(V{1.0} == 1));
            assert(!(1 == V{1.0}));
            assert(!(V{1} == 1.0));
            assert(!(1.0 == V{1}));
        }
        {
            using V = variant< int, double >;
            assert(V{1} < V{2});
            assert(V{1.0} < V{2.0});
            assert(!(V{1} < V{0}));
            assert(!(V{1.0} < V{0.0}));
            assert(1 < V{2});
            assert(V{1} < 2);
            assert(V{1.0} < 2.0);
            assert(1.0 < V{2.0});
            assert(!(V{1} < 0));
            assert(!(1 < V{0}));
            assert(!(V{1.0} < 0.0));
            assert(!(1.0 < V{0.0}));
        }
        {
            struct A { A(int &, int) {} };
            struct B { B(int const &, int) {} };
            using V = variant< A, B >;
            V v{1, 2};
            assert(v.active< B >());
            int i{1};
            V w{i, 2};
            assert(w.active< A >());
        }
        {
            variant< char const * > v;
            assert(static_cast< char const * >(v) == nullptr);
        }
        {
            struct A {};
            struct B { B(B &&) = default; B(B const &) = delete; B & operator = (B &&) = default; void operator = (B const &) = delete; };
            variant< A, B > v;
            assert(v.active< A >());
            v = B{};
            assert(v.active< B >());
        }
        {
            struct A {};
            using V = variant< A >;
            visitor0 p_;
            visitor0 const cp_{};
            V v;
            V const cv;
            assert(v.apply_visitor(p_)    == std::make_tuple(false, false, true));
            assert(v.apply_visitor(cp_)   == std::make_tuple(true,  false, true));
            assert(cv.apply_visitor(p_)   == std::make_tuple(false, true,  true));
            assert(cv.apply_visitor(cp_)  == std::make_tuple(true,  true,  true));
            assert(V().apply_visitor(p_)  == std::make_tuple(false, false, false));
            assert(V().apply_visitor(cp_) == std::make_tuple(true,  false, false));
        }
        {
            struct A {};
            using V = variant< A >;
            visitor1 p_;
            visitor1 const cp_{};
            V v;
            V const cv;
            assert(v.apply_visitor(p_)           == std::make_tuple(false, true,  false, true));
            assert(v.apply_visitor(cp_)          == std::make_tuple(true,  true,  false, true));
            assert(v.apply_visitor(visitor1{})   == std::make_tuple(false, false, false, true));
            assert(cv.apply_visitor(p_)          == std::make_tuple(false, true,  true,  true));
            assert(cv.apply_visitor(cp_)         == std::make_tuple(true,  true,  true,  true));
            assert(cv.apply_visitor(visitor1{})  == std::make_tuple(false, false, true,  true));
            assert(V().apply_visitor(p_)         == std::make_tuple(false, true,  false, false));
            assert(V().apply_visitor(cp_)        == std::make_tuple(true,  true,  false, false));
            assert(V().apply_visitor(visitor1{}) == std::make_tuple(false, false, false, false));
        }
        { // multivisitation with forwarding
            struct A {};
            struct B {};
            using V = variant< A, B >;
            visitor2 v_;
            visitor2 const c_{};
            assert(apply_visitor(v_, V{B{}}) == std::make_tuple(false, std::tie(typeid(B)), false, false));
            assert(apply_visitor(c_, V{B{}}) == std::make_tuple(true, std::tie(typeid(B)), false, false));
            assert(apply_visitor(visitor2{}, V{B{}}) == std::make_tuple(false, std::tie(typeid(B)), false, false));
            assert(apply_visitor(v_, V{}, V{B{}}) == std::make_tuple(false, std::tie(typeid(A)), false, false, std::tie(typeid(B)), false, false));
            assert(apply_visitor(c_, V{}, V{B{}}) == std::make_tuple(true, std::tie(typeid(A)), false, false, std::tie(typeid(B)), false, false));
            assert(apply_visitor(visitor2{}, V{}, V{B{}}) == std::make_tuple(false, std::tie(typeid(A)), false, false, std::tie(typeid(B)), false, false));
            // forwarding
            enum class op { eq, lt, gt, nge };
            assert(apply_visitor(v_, V{}, op::eq, V{B{}}) == std::make_tuple(false, std::tie(typeid(A)), false, false, std::tie(typeid(op)), false, false, std::tie(typeid(B)), false, false));
            assert(apply_visitor(c_, V{}, op::eq, V{B{}}) == std::make_tuple(true, std::tie(typeid(A)), false, false, std::tie(typeid(op)), false, false, std::tie(typeid(B)), false, false));
            assert(apply_visitor(visitor2{}, V{}, op::eq, V{B{}}) == std::make_tuple(false, std::tie(typeid(A)), false, false, std::tie(typeid(op)), false, false, std::tie(typeid(B)), false, false));
        }
        { // delayed visitation
            visitor2 v_;
            visitor2 const c_{};

            auto d0 = apply_visitor(v_);
            auto const d1 = apply_visitor(v_);
            auto d2 = apply_visitor(c_);
            auto const d3 = apply_visitor(c_);
            auto d4 = apply_visitor(visitor2{});
            auto const d5 = apply_visitor(visitor2{});

            struct A {};
            struct B {};
            using V = variant< A, B >;
            V v;
            V const c(B{});

            assert(d0(v) ==   std::make_tuple(false, std::tie(typeid(A)), false, true ));
            assert(d0(c) ==   std::make_tuple(false, std::tie(typeid(B)), true,  true ));
            assert(d0(V{}) == std::make_tuple(false, std::tie(typeid(A)), false, false));

            assert(d1(v) ==   std::make_tuple(false, std::tie(typeid(A)), false, true ));
            assert(d1(c) ==   std::make_tuple(false, std::tie(typeid(B)), true,  true ));
            assert(d1(V{}) == std::make_tuple(false, std::tie(typeid(A)), false, false));

            assert(d2(v) ==   std::make_tuple(true,  std::tie(typeid(A)), false, true ));
            assert(d2(c) ==   std::make_tuple(true,  std::tie(typeid(B)), true,  true ));
            assert(d2(V{}) == std::make_tuple(true,  std::tie(typeid(A)), false, false));

            assert(d3(v) ==   std::make_tuple(true,  std::tie(typeid(A)), false, true ));
            assert(d3(c) ==   std::make_tuple(true,  std::tie(typeid(B)), true,  true ));
            assert(d3(V{}) == std::make_tuple(true,  std::tie(typeid(A)), false, false));

            assert(d4(v) ==   std::make_tuple(false, std::tie(typeid(A)), false, true ));
            assert(d4(c) ==   std::make_tuple(false, std::tie(typeid(B)), true,  true ));
            assert(d4(V{}) == std::make_tuple(false, std::tie(typeid(A)), false, false));

            // to use mutable visitor it is mandatory to have non-const delayed visitor instance
            assert(d5(v) ==   std::make_tuple(true,  std::tie(typeid(A)), false, true ));
            assert(d5(c) ==   std::make_tuple(true,  std::tie(typeid(B)), true,  true ));
            assert(d5(V{}) == std::make_tuple(true,  std::tie(typeid(A)), false, false));
        }
        { // delayed visitation
            visitor3 v_;
            visitor3 const cv_{};

            auto d = apply_visitor(v_);
            auto const cd = apply_visitor(v_);
            auto dcv = apply_visitor(cv_);
            auto const cdcv = apply_visitor(cv_);
            auto dmv = apply_visitor(visitor3{});
            auto const cdmv = apply_visitor(visitor3{});

            struct A {};
            using V = variant< A >;
            V v;
            V const cv{};

            assert(d(v)                           == std::make_tuple(false, true,  false, true ));
            assert(cd(v)                          == std::make_tuple(false, true,  false, true )); // ! true, true, false, true
            assert(apply_visitor(v_)(v)           == std::make_tuple(false, false, false, true ));

            assert(dcv(v)                         == std::make_tuple(true,  true,  false, true ));
            assert(cdcv(v)                        == std::make_tuple(true,  true,  false, true ));
            assert(apply_visitor(cv_)(v)          == std::make_tuple(true,  false, false, true )); // if visitor have not `const &&` version of `operator ()`, then `const &` chosen

            assert(dmv(v)                         == std::make_tuple(false, true,  false, true ));
            assert(cdmv(v)                        == std::make_tuple(true,  true,  false, true ));
            assert(apply_visitor(visitor3{})(v)   == std::make_tuple(false, false, false, true ));

            assert(d(cv)                          == std::make_tuple(false, true,  true,  true ));
            assert(cd(cv)                         == std::make_tuple(false, true,  true,  true )); // ! true, true, true, true
            assert(apply_visitor(v_)(cv)          == std::make_tuple(false, false, true,  true ));

            assert(dcv(cv)                        == std::make_tuple(true,  true,  true,  true ));
            assert(cdcv(cv)                       == std::make_tuple(true,  true,  true,  true ));
            assert(apply_visitor(cv_)(cv)         == std::make_tuple(true,  false, true,  true ));

            assert(dmv(cv)                        == std::make_tuple(false, true,  true,  true ));
            assert(cdmv(cv)                       == std::make_tuple(true,  true,  true,  true ));
            assert(apply_visitor(visitor3{})(cv)  == std::make_tuple(false, false, true,  true ));

            assert(d(V{})                         == std::make_tuple(false, true,  false, false));
            assert(cd(V{})                        == std::make_tuple(false, true,  false, false)); // ! true, true, false, false
            assert(apply_visitor(v_)(V{})         == std::make_tuple(false, false, false, false));

            assert(dcv(V{})                       == std::make_tuple(true,  true,  false, false));
            assert(cdcv(V{})                      == std::make_tuple(true,  true,  false, false));
            assert(apply_visitor(cv_)(V{})        == std::make_tuple(true,  false, false, false));

            assert(dmv(V{})                       == std::make_tuple(false, true,  false, false));
            assert(cdmv(V{})                      == std::make_tuple(true,  true,  false, false));
            assert(apply_visitor(visitor3{})(V{}) == std::make_tuple(false, false, false, false));
        }
        {
            struct A {};
            struct B {};
            struct C {};
            using V = variant< A, B, C >;
            static_assert((std::is_same< variant< std::reference_wrapper< A >, std::reference_wrapper< B >, std::reference_wrapper< C > >, V::wrap< std::reference_wrapper > >{}), "!");
            static_assert((std::is_same< std::tuple< A, B, C >, V::engage< std::tuple > >{}), "!");
            static_assert((std::is_same< std::tuple< std::reference_wrapper< A >, std::reference_wrapper< B >, std::reference_wrapper< C > >, V::wrap< std::reference_wrapper, std::tuple > >{}), "!");
        }
        {
            using V = variant< int, double >;
            std::stringstream ss_;
            ss_.str("1");
            V v = 2;
            ss_ >> v;
            assert(v == 1);
            v = 3.5;
            ss_.str();
            ss_.clear();
            ss_ << v;
            assert(ss_.str() == "3.5");
        }
        {
            struct A { A(int) {} };
            struct B {};
            auto v = make_variant< variant< A, B > >(1.0);
            assert(v.active< A >());
        }
        {
            auto const l0 = [] (auto const &) { return 0; };
            auto const l1 = [] (auto &) { return 1; };
            auto const l2 = [] (auto const &&) { return 2; };
            auto const l3 = [] (auto &&) { return 3; };
            {
                struct A {};
                using V = variant< A >;
                V v;
                V const c{};
                auto const l = compose_visitors(l0, l1, l2, l3);
                assert(0 == c.apply_visitor(l));
                assert(1 == v.apply_visitor(l));
                assert(2 == std::move(c).apply_visitor(l));
                assert(3 == std::move(v).apply_visitor(l));
            }
            {
                struct A
                {

                    A(int) { ; }
                    A() = delete;
                    A(A const &) = delete;
                    A(A &) = delete;
                    A(A &&) = delete;
                    A & operator = (A const &) = delete;
                    A & operator = (A &) = delete;
                    A & operator = (A &&) = delete;

                };
                A av{0};
                A const ac{1};
                {
                    auto const l = compose_visitors(l0, l1, l2, l3);
                    assert(0 == l(ac));
                    assert(1 == l(av));
                    assert(2 == l(std::move(ac)));
                    assert(3 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0);
                    assert(0 == l(ac));
                    assert(0 == l(av));
                    assert(0 == l(std::move(ac)));
                    assert(0 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l1);
                    assert(1 == l(ac));
                    assert(1 == l(av));
                    assert(1 == l(std::move(ac)));
                    //assert(1 == l(std::move(v)));
                }
                {
                    auto const l = compose_visitors(l2);
                    //assert(2 == l(c));
                    //assert(2 == l(v));
                    assert(2 == l(std::move(ac)));
                    assert(2 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l3);
                    assert(3 == l(ac));
                    assert(3 == l(av));
                    assert(3 == l(std::move(ac)));
                    assert(3 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l1);
                    assert(0 == l(ac));
                    assert(1 == l(av));
                    assert(0 == l(std::move(ac)));
                    assert(0 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l2);
                    assert(0 == l(ac));
                    assert(0 == l(av));
                    assert(2 == l(std::move(ac)));
                    assert(2 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l3);
                    assert(0 == l(ac));
                    assert(3 == l(av));
                    assert(3 == l(std::move(ac)));
                    assert(3 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l1, l2);
                    assert(1 == l(ac));
                    assert(1 == l(av));
                    assert(2 == l(std::move(ac)));
                    assert(2 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l1, l3);
                    assert(1 == l(ac));
                    assert(1 == l(av));
                    assert(3 == l(std::move(ac)));
                    assert(3 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l2, l3);
                    assert(3 == l(ac));
                    assert(3 == l(av));
                    assert(2 == l(std::move(ac)));
                    assert(3 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l1, l2);
                    assert(0 == l(ac));
                    assert(1 == l(av));
                    assert(2 == l(std::move(ac)));
                    assert(2 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l1, l3);
                    assert(0 == l(ac));
                    assert(1 == l(av));
                    assert(3 == l(std::move(ac)));
                    assert(3 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l2, l3);
                    assert(0 == l(ac));
                    assert(3 == l(av));
                    assert(2 == l(std::move(ac)));
                    assert(3 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l1, l2, l3);
                    assert(1 == l(ac));
                    assert(1 == l(av));
                    assert(2 == l(std::move(ac)));
                    assert(3 == l(std::move(av)));
                }
            }
            {
                auto const l = compose_visitors(l0, l1, l2, l3);
                struct A {};
                A a;
                A const c{};
                {
                    struct R {};
                    auto const rvl = add_result_type< R >(l);
                    static_assert((std::is_same< decltype(rvl)::result_type, R >{}));
                    assert(0 == rvl(c));
                    assert(1 == rvl(a));
                    assert(2 == rvl(std::move(c)));
                    assert(3 == rvl(A{}));
                }
                {
                    auto const la = compose_visitors(l, [] (A const &) { return -1; });
                    assert(-1 == la(c));
                    assert(1 == la(a));
                    assert(2 == la(std::move(c)));
                    assert(3 == la(A{}));
                }
                {
                    struct V { auto operator () (A const &) && { return -11; } };
                    auto lam = compose_visitors(l, V{});
                    assert(0 == lam(c));
                    assert(-11 == std::move(lam)(c));
                }
            }
        }
    }
    {
        assert((test< ROWS, COLS >())); // 8 seconds (Release build) for COLS=5 ROWS=5 on Intel(R) Xeon(R) CPU E5-1650 0 @ 3.20GHz
    }
    return EXIT_SUCCESS;
}
