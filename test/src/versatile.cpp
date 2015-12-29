#include "test/versatile.hpp"
#include "test/common.hpp"
#include "test/wrappers.hpp"
#include "test/visit.hpp"

#include <cstdlib>

int
main()
{
    using ::versatile::identity;
    using ::test::aggregate;
    using ::test::recursive_wrapper;
    using ::test::literal_type;
    using ::test::check_indexing;
    using ::test::check_invariants;
    using ::test::check_triviality;
    using ::test::check_utility;
    using ::test::check_destructible;
    using ::test::perferct_forwarding;
    { // versatile
        using ::versatile::versatile;
        {
            ASSERT ((check_indexing< identity,          versatile >::run()));
            ASSERT ((check_indexing< aggregate,         versatile >::run()));
            //CHECK ((check_indexing< recursive_wrapper, versatile >::run()));
        }
        {
            ASSERT ((check_invariants< identity,  versatile >::run()));
            ASSERT ((check_invariants< aggregate, versatile >::run()));
        }
        {
            ASSERT ((check_triviality< identity,  versatile >::run()));
            ASSERT ((check_triviality< aggregate, versatile >::run()));
        }
        {
            ASSERT ((check_utility< identity,  versatile >::run()));
            ASSERT ((check_utility< aggregate, versatile >::run()));
        }
        {
            CHECK ((check_destructible< identity,          versatile >::run()));
            CHECK ((check_destructible< aggregate,         versatile >::run()));
            CHECK ((check_destructible< recursive_wrapper, versatile >::run()));
        }
        {
            ASSERT ((perferct_forwarding< literal_type, versatile, identity,  2, 2 >::run()));
            ASSERT ((perferct_forwarding< literal_type, versatile, aggregate, 2, 2 >::run()));
        }
    }
    return EXIT_SUCCESS;
}

#if 0

namespace versatile
{ // little extension (candidates to include in library)

// value_or
template< typename lhs, typename rhs,
          typename result_type = unwrap_type_t< rhs > >
constexpr
std::enable_if_t< (is_visitable_v< unwrap_type_t< lhs > > && !is_visitable_v< result_type >), result_type >
operator || (lhs && _lhs, rhs && _rhs) noexcept
{
    if (_lhs.template active< result_type >()) {
        return static_cast< result_type >(std::forward< lhs >(_lhs));
    } else {
        return std::forward< rhs >(_rhs);
    }
}

template< typename lhs, typename rhs,
          typename result_type = unwrap_type_t< lhs > >
constexpr
std::enable_if_t< (!is_visitable_v< result_type > && is_visitable_v< unwrap_type_t< rhs > >), result_type >
operator || (lhs && _lhs, rhs && _rhs) noexcept
{
    return (std::forward< rhs >(_rhs) || std::forward< lhs >(_lhs));
}

template< typename lhs, typename rhs >
std::enable_if_t< (is_visitable_v< unwrap_type_t< lhs > > && is_visitable_v< unwrap_type_t< rhs > >) >
operator || (lhs && _lhs, rhs && _rhs) = delete;

} // namespace versatile

namespace test
{

inline int f() { return 1; }
inline int g() { return 2; }

template< typename T >
using AW = versatile::aggregate_wrapper< T >;

template< typename T >
using RW = versatile::recursive_wrapper< T >;

struct R;
struct A {};
using V = versatile::variant< A, RW< R > >;
struct R
        : V
{
    using V::V;
    using V::operator =;
};

} // namespace test

int
main()
{
    using namespace test;
    using versatile::visit;
    using versatile::multivisit;
    using versatile::compose_visitors;
    using versatile::in_place;
    using versatile::variant;
    using versatile::versatile;
    {
        {
            using V = versatile< int >;
            V w; // equivalent to default construction
            CHECK(static_cast< int >(w) == int{});
            V v{2};
            CHECK(static_cast< int >(v) == 2);
        }
        {
            using V = versatile< std::true_type, std::false_type >;
            V t;
            CHECK(static_cast< std::true_type >(t) == std::true_type{});
            V f{std::false_type{}};
            CHECK(static_cast< std::false_type >(f) == std::false_type{});
        }
        {
            using V = variant< std::true_type, std::false_type >;
            V t;
            CHECK(static_cast< std::true_type >(t) == std::true_type{});
            V f{std::false_type{}};
            CHECK(static_cast< std::false_type >(f) == std::false_type{});
        }
#if 0
        { // trivial
            using V = versatile< int, double >;
            //SA(std::is_trivially_copy_constructible< V >{}); // ???
            //SA(std::is_trivially_move_constructible< V >{}); // ???
            SA(std::is_trivially_copy_assignable< V >{});
            SA(std::is_trivially_move_assignable< V >{});
            V v{1.0};
            CHECK(v.active< double >());
            V u{v};
            CHECK(u.active< double >());
            V w{1};
            v = w;
            CHECK(v.active< int >());
        }
#endif
#if 0
        { // non-standard layout (really UB http://talesofcpp.fusionfenix.com/post-21/)
            struct A { int i; };
            struct B : A { A a; B(int i, int j) : A{i}, a{j} { ; } };
            SA(!std::is_standard_layout< B >{});
            using V = versatile< A, B >;
            SA(!std::is_standard_layout< V >{});
            V v{B{1, 2}};
            CHECK(v.active< B >());
            CHECK(static_cast< B >(v).A::i == 1);
            CHECK(static_cast< B >(v).a.i == 2);
        }
        { // non-standard layout (really UB http://talesofcpp.fusionfenix.com/post-21/)
            struct A { int i; A(int j) : i(j) { ; } };
            struct B : A { using A::A; };
            struct C : A { using A::A; };
            struct D : B, C { D(int i, int j) : B{i}, C{j} { ; } };
            SA(!std::is_standard_layout< D >{});
            using V = versatile< A, B, C, D >;
            SA(!std::is_standard_layout< V >{});
            V v{D{1, 2}};
            CHECK(v.active< D >());
            CHECK(static_cast< D >(v).B::i == 1);
            CHECK(static_cast< D >(v).C::i == 2);
        }
#endif
        {
            using V = variant< int >;
            V v{2};
            CHECK(static_cast< int >(v) == 2);
        }
        { // value_or
            using V = variant< double, int >;
            {
                V D{1.0};
                int i = D || 2;
                CHECK(i == 2);
                V I{1};
                int j = I || 2;
                CHECK(j == 1);
            }
            {
                int i = 2 || V{1.0};
                CHECK(i == 2);
                int j = 2 || V{1} ;
                CHECK(j == 1);
            }
        }
        {
            using std::swap;
            using V = variant< int >;
            SA(V::width == 1, "V::width != 1");
            V v;
            SA(1 == variadic_index< V, int >{});
            CHECK(v.active< int >());
            CHECK(static_cast< int >(v) == int{});
            V w(222);
            CHECK(static_cast< int >(w) == 222);
            v = w;
            CHECK(static_cast< int >(v) == 222);
            CHECK(v == w);
            CHECK(!(v < w));
            CHECK(!(w < v));
            ++static_cast< int & >(v);
            CHECK(w < v);
            CHECK(!(v < w));
            CHECK(!(v == w));
            CHECK(static_cast< int >(w) == 222);
            CHECK(static_cast< int >(v) == 223);
            swap(v, w);
            CHECK(static_cast< int >(w) == 223);
            CHECK(static_cast< int >(v) == 222);
            swap(w, v);
            CHECK(static_cast< int >(w) == 222);
            CHECK(static_cast< int >(v) == 223);
            V u = w;
            CHECK(static_cast< int >(u) == 222);
            CHECK(w.active< int >());
            SA(variadic_index< V, int >{} == 1);
            CHECK(u.active< int >());
        }
        {
            using V = variant< int, float, double, long double >;
            CHECK((V{}.which() == variadic_index< V, int >{}));
            CHECK((V{0}.which() == variadic_index< V, int >{}));
            CHECK((V{1.0f}.which() == variadic_index< V, float >{}));
            CHECK((V{2.0}.which() == variadic_index< V, double >{}));
            CHECK((V{3.0L}.which() == variadic_index< V, long double >{}));
            V i;
            CHECK(i.active< int >());
            V j = 1;
            CHECK(j.active< int >());
            V f = 1.0f;
            CHECK(f.active< float >());
            V d = 2.0;
            CHECK(d.active< double >());
            V l = 3.0L;
            CHECK(l.active< long double >());
            i = l;
            CHECK(i.active< long double >());
            l = d;
            CHECK(l.active< double >());
            d = f;
            CHECK(d.active< float >());
            f = j;
            CHECK(f.active< int >());
            using std::swap;
            swap(d, j);
            CHECK(d.active< int >());
            CHECK(j.active< float >());
        }
        {
            struct A { A() = delete; };
            struct B {};
            using V =  variant< A, B >;
            V v;
            CHECK(v.active< B >());
            v = A{};
            CHECK(v.active< A >());
            V w{A{}};
            CHECK(w.active< A >());
        }
        { // incomplete
            struct A {};
            struct B;
            using V = variant< A, RW< B > >;
            V v;
            CHECK(v.active< A >());
            struct B {}; // if declared but not defined then there is compilation error in std::unique_ptr destructor
            v = B{};
            CHECK(v.active< B >());
            CHECK(v.active< RW< B > >());
        }
        { // recursive (composition)
            struct R;
            struct A {};
            using V = variant< A, RW< R > >;
            V v;
            CHECK(v.active< A >());
            struct R { V v; };
            v = R{};
            CHECK(v.active< R >());
            CHECK(v.active< RW< R > >());
            CHECK(static_cast< R & >(v).v.active< A >());
        }
        { // recursive (inheritance)
            struct R;
            struct A {};
            using V = variant< A, RW< R > >;
            V u;
            CHECK(u.active< A >());
            struct R : V { using V::V; using V::operator =; };
            u = R{};
            CHECK(u.active< R >());
            CHECK(u.active< RW< R > >());
            R v;
            CHECK(v.active< A >());
            v = R{};
            CHECK(v.active< A >());
            R w{R{}};
            CHECK(w.active< A >());
            R x{V{R{}}};
            CHECK(x.active< R >());
            CHECK(x.active< RW< R > >());
        }
        { // exact
            using V = variant< int, bool >;
            CHECK(V{} == V{});
            CHECK(V{} == int{});
            CHECK(int{} == V{});
            CHECK(V{1} == 1);
            CHECK(1 == V{1});
            CHECK(V{false} == false);
            CHECK(true == V{true});
        }
        { // relational
            using V = variant< int, double >;
            CHECK(V{1} < V{2});
            CHECK(V{1.0} < V{2.0});
            CHECK(!(V{1} < V{0}));
            CHECK(!(V{1.0} < V{0.0}));
            CHECK(1 < V{2});
            CHECK(V{1} < 2);
            CHECK(V{1.0} < 2.0);
            CHECK(1.0 < V{2.0});
            CHECK(!(V{1} < 0));
            CHECK(!(1 < V{0}));
            CHECK(!(V{1.0} < 0.0));
            CHECK(!(1.0 < V{0.0}));
        }
        {
            struct A
            {
                A(int j) : i(j) { ; }
                A(A && a) : i(a.i) { a.i = 0; }
                void operator = (A && a) { i = a.i; a.i = 0; }
                operator int () const { return i; }
            private :
                int i;
            };
            using V = variant< A >;
            V x{in_place, 1};
            V y{in_place, 2};
            y = std::move(x);
            CHECK(x == A{0});
            CHECK(y == A{1});
        }
        {
            struct A { A(int &, int) {} };
            struct B { B(int const &, int) {} };
            using V = variant< A, B >;
            V v{in_place, 1, 2};
            CHECK(v.active< B >());
            int i{1};
            V w{in_place, i, 2};
            CHECK(w.active< A >());
        }
        {
            variant< char const * > v;
            CHECK(static_cast< char const * >(v) == nullptr);
        }
        {
            struct A {};
            struct B { B(B &&) = default; B(B const &) = delete; B & operator = (B &&) = default; void operator = (B const &) = delete; };
            variant< B, A > v;
            CHECK(v.active< A >());
            v = B{};
            CHECK(v.active< B >());
        }
        {
            struct A { A(double) { ; } };
            using V = variant< A, int >;
            CHECK(V(in_place, int{0}).active< A >());
            CHECK(V{int{0}}.active< int >());
        }
        {
            struct B;
            struct A { A(B const &) { ; } };
            struct B {};
            using V = variant< A, B >;
            CHECK(V{}.active< B >());
            CHECK(V{B{}}.active< B >());
            CHECK((V{in_place, B{}}.active< A >()));
        }
        {
            static int counter = 0;
            struct A {};
            struct B { int i; B(int j) : i(j) { ++counter; } B() = delete; B(B const &) = delete; B(B &&) = delete; };
            struct C {};
            B b{1};
            CHECK(counter == 1);
            using V = variant< A, B, C >;
            V v;
            CHECK(v.active< A >());
            CHECK(counter == 1);
            v.emplace(2);
            CHECK(v.active< B >());
            CHECK(static_cast< B const & >(v).i == 2);
            CHECK(counter == 2);
            v.emplace(1);
            CHECK(v.active< B >());
            CHECK(static_cast< B const & >(v).i == 1);
            CHECK(counter == 3);
        }
        {
            struct A {};
            using V = variant< RW< A > >;
            visitor0 p_;
            visitor0 const cp_{};
            V v;
            V const cv;
            using B3 = std::array< bool, 3 >;
            CHECK((visit(p_, v)    == B3{{false, false, true }}));
            CHECK((visit(cp_, v)   == B3{{true,  false, true }}));
            CHECK((visit(p_, cv)   == B3{{false, true,  true }}));
            CHECK((visit(cp_, cv)  == B3{{true,  true,  true }}));
            CHECK((visit(p_, V())  == B3{{false, false, false}}));
            CHECK((visit(cp_, V()) == B3{{true,  false, false}}));
        }
        {
            struct A {};
            using V = variant< RW< A > >;
            visitor1 p_;
            visitor1 const cp_{};
            V v;
            V const cv{};
            using B4 = std::array< bool, 4 >;
            CHECK((visit(p_, v)           == B4{{false, true,  false, true }}));
            CHECK((visit(cp_, v)          == B4{{true,  true,  false, true }}));
            CHECK((visit(visitor1{}, v)   == B4{{false, false, false, true }}));
            CHECK((visit(p_, cv)          == B4{{false, true,  true,  true }}));
            CHECK((visit(cp_, cv)         == B4{{true,  true,  true,  true }}));
            CHECK((visit(visitor1{}, cv)  == B4{{false, false, true,  true }}));
            CHECK((visit(p_, V())         == B4{{false, true,  false, false}}));
            CHECK((visit(cp_, V())        == B4{{true,  true,  false, false}}));
            CHECK((visit(visitor1{}, V()) == B4{{false, false, false, false}}));
        }
        { // multivisitation with forwarding
            struct A {};
            struct B {};
            using V = variant< RW< A >, RW< B > >;
            visitor2 v_;
            visitor2 const c_{};
            CHECK(multivisit(v_, V{B{}}) == std::make_tuple(false, std::tie(typeid(B)), false, false));
            CHECK(multivisit(c_, V{B{}}) == std::make_tuple(true, std::tie(typeid(B)), false, false));
            CHECK(multivisit(visitor2{}, V{B{}}) == std::make_tuple(false, std::tie(typeid(B)), false, false));
            CHECK(multivisit(v_, V{}, V{B{}}) == std::make_tuple(false, std::tie(typeid(A)), false, false, std::tie(typeid(B)), false, false));
            CHECK(multivisit(c_, V{}, V{B{}}) == std::make_tuple(true, std::tie(typeid(A)), false, false, std::tie(typeid(B)), false, false));
            CHECK(multivisit(visitor2{}, V{}, V{B{}}) == std::make_tuple(false, std::tie(typeid(A)), false, false, std::tie(typeid(B)), false, false));
            // forwarding
            enum class op { eq, lt, gt, nge };
            CHECK(multivisit(v_, V{}, op::eq, V{B{}}) == std::make_tuple(false, std::tie(typeid(A)), false, false, std::tie(typeid(op)), false, false, std::tie(typeid(B)), false, false));
            CHECK(multivisit(c_, V{}, op::eq, V{B{}}) == std::make_tuple(true, std::tie(typeid(A)), false, false, std::tie(typeid(op)), false, false, std::tie(typeid(B)), false, false));
            CHECK(multivisit(visitor2{}, V{}, op::eq, V{B{}}) == std::make_tuple(false, std::tie(typeid(A)), false, false, std::tie(typeid(op)), false, false, std::tie(typeid(B)), false, false));
        }
        { // delayed visitation
            visitor2 v_;
            visitor2 const c_{};

            auto d0 = visit(v_);
            auto const d1 = visit(v_);
            auto d2 = visit(c_);
            auto const d3 = visit(c_);
            auto d4 = visit(visitor2{});
            auto const d5 = visit(visitor2{});

            struct A {};
            struct B {};
            using V = variant< RW< A >, RW< B > >;
            V v;
            V const c(B{});

            CHECK(d0(v) ==   std::make_tuple(false, std::tie(typeid(A)), false, true ));
            CHECK(d0(c) ==   std::make_tuple(false, std::tie(typeid(B)), true,  true ));
            CHECK(d0(V{}) == std::make_tuple(false, std::tie(typeid(A)), false, false));

            CHECK(d1(v) ==   std::make_tuple(true, std::tie(typeid(A)), false, true ));
            CHECK(d1(c) ==   std::make_tuple(true, std::tie(typeid(B)), true,  true ));
            CHECK(d1(V{}) == std::make_tuple(true, std::tie(typeid(A)), false, false));

            CHECK(d2(v) ==   std::make_tuple(true,  std::tie(typeid(A)), false, true ));
            CHECK(d2(c) ==   std::make_tuple(true,  std::tie(typeid(B)), true,  true ));
            CHECK(d2(V{}) == std::make_tuple(true,  std::tie(typeid(A)), false, false));

            CHECK(d3(v) ==   std::make_tuple(true,  std::tie(typeid(A)), false, true ));
            CHECK(d3(c) ==   std::make_tuple(true,  std::tie(typeid(B)), true,  true ));
            CHECK(d3(V{}) == std::make_tuple(true,  std::tie(typeid(A)), false, false));

            CHECK(d4(v) ==   std::make_tuple(false, std::tie(typeid(A)), false, true ));
            CHECK(d4(c) ==   std::make_tuple(false, std::tie(typeid(B)), true,  true ));
            CHECK(d4(V{}) == std::make_tuple(false, std::tie(typeid(A)), false, false));

            CHECK(d5(v) ==   std::make_tuple(true,  std::tie(typeid(A)), false, true ));
            CHECK(d5(c) ==   std::make_tuple(true,  std::tie(typeid(B)), true,  true ));
            CHECK(d5(V{}) == std::make_tuple(true,  std::tie(typeid(A)), false, false));
        }
        { // delayed visitation
            visitor3 v_;
            visitor3 const cv_{};

            auto d = visit(v_);
            auto const cd = visit(v_);
            auto dcv = visit(cv_);
            auto const cdcv = visit(cv_);
            auto dmv = visit(visitor3{});
            auto const cdmv = visit(visitor3{});

            struct A {};
            using V = variant< RW< A > >;
            V v;
            V const cv{};

            CHECK(d(v)                    == std::make_tuple(false, true,  false, true ));
            CHECK(cd(v)                   == std::make_tuple(true,  true,  false, true ));
            CHECK(visit(v_)(v)            == std::make_tuple(false, false, false, true ));

            CHECK(dcv(v)                  == std::make_tuple(true,  true,  false, true ));
            CHECK(cdcv(v)                 == std::make_tuple(true,  true,  false, true ));
            CHECK(visit(cv_)(v)           == std::make_tuple(true,  false, false, true ));

            CHECK(dmv(v)                  == std::make_tuple(false, true,  false, true ));
            CHECK(cdmv(v)                 == std::make_tuple(true,  true,  false, true ));
            CHECK(visit(visitor3{})(v)    == std::make_tuple(false, false, false, true ));

            CHECK(d(cv)                   == std::make_tuple(false, true,  true,  true ));
            CHECK(cd(cv)                  == std::make_tuple(true,  true,  true,  true ));
            CHECK(visit(v_)(cv)           == std::make_tuple(false, false, true,  true ));

            CHECK(dcv(cv)                 == std::make_tuple(true,  true,  true,  true ));
            CHECK(cdcv(cv)                == std::make_tuple(true,  true,  true,  true ));
            CHECK(visit(cv_)(cv)          == std::make_tuple(true,  false, true,  true ));

            CHECK(dmv(cv)                 == std::make_tuple(false, true,  true,  true ));
            CHECK(cdmv(cv)                == std::make_tuple(true,  true,  true,  true ));
            CHECK(visit(visitor3{})(cv)   == std::make_tuple(false, false, true,  true ));

            CHECK(d(V{})                  == std::make_tuple(false, true,  false, false));
            CHECK(cd(V{})                 == std::make_tuple(true,  true,  false, false));
            CHECK(visit(v_)(V{})          == std::make_tuple(false, false, false, false));

            CHECK(dcv(V{})                == std::make_tuple(true,  true,  false, false));
            CHECK(cdcv(V{})               == std::make_tuple(true,  true,  false, false));
            CHECK(visit(cv_)(V{})         == std::make_tuple(true,  false, false, false));

            CHECK(dmv(V{})                == std::make_tuple(false, true,  false, false));
            CHECK(cdmv(V{})               == std::make_tuple(true,  true,  false, false));
            CHECK(visit(visitor3{})(V{})  == std::make_tuple(false, false, false, false));
        }
        {
            using V = variant< RW< int >, double >;
            std::stringstream ss_;
            ss_.str("1");
            V v = 2;
            ss_ >> v;
            CHECK(v == 1);
            v = 3.5;
            ss_.str();
            ss_.clear();
            ss_ << v;
            CHECK(ss_.str() == "3.5");
        }
        { // invoke
            struct A { int operator () (int) { return 0; } bool operator () (double) { return true; } };
            struct B { int operator () (int) { return 1; } bool operator () (double) { return false; } };
            using V = variant< A, B >;
            V v;
            SA(std::is_same< int, decltype(invoke(v, 0)) >{});
            SA(std::is_same< bool, decltype(invoke(v, 0.0)) >{});
            CHECK(invoke(v, 0) == 0);
            CHECK(invoke(v, 1.0) == true);
            v = B{};
            CHECK(invoke(v, 0) == 1);
            CHECK(invoke(v, 1.0) == false);
        }
        { // invoke
            auto a = [] (auto &&... _values) -> int { return +static_cast< int >(sizeof...(_values)); };
            auto b = [] (auto &&... _values) -> int { return -static_cast< int >(sizeof...(_values)); };
            using V = variant< decltype(a), decltype(b) >;
            V v = a;
            SA(std::is_same< int, decltype(invoke(v, 0)) >{});
            CHECK(invoke(v, 0, 1.0f, 2.0, 3.0L, true) == +5);
            CHECK(invoke(v) == 0);
            CHECK(invoke(v, nullptr) == 1);
            v = std::move(b);
            CHECK(invoke(v, 0, 1.0f, 2.0, 3.0L, true) == -5);
            CHECK(invoke(v) == 0);
            CHECK(invoke(v, nullptr) == -1);
        }
        {
            auto const l0 = [] (auto const &) { return 0; };
            auto const l1 = [] (auto &) { return 1; };
            auto const l2 = [] (auto const &&) { return 2; };
            auto const l3 = [] (auto &&) { return 3; };
            auto const l4 = [] (auto && arg) -> std::enable_if_t< std::is_rvalue_reference< decltype(arg) >{}, int > { return 4; };
            {
                struct A {};
                using V = variant< A >;
                V v;
                V const c{};
                auto const l = compose_visitors(l0, l1, l2, l3, [] { ; });
                CHECK(0 == visit(l, c));
                CHECK(1 == visit(l, v));
                CHECK(2 == visit(l, std::move(c)));
                CHECK(3 == visit(l, std::move(v)));
                CHECK(3 == visit(l, std::move(v)));
                SA(std::is_void< decltype(visit(l)()) >{});
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
                using V = variant< RW< A > >;
                V av{in_place, 0};
                V const ac{in_place, 1};
                {
                    auto const l = compose_visitors(l0, l1, l2, l3);
                    CHECK(0 == l(ac));
                    CHECK(1 == l(av));
                    CHECK(2 == l(std::move(ac)));
                    CHECK(3 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l1, l2, l4);
                    CHECK(0 == l(ac));
                    CHECK(1 == l(av));
                    CHECK(2 == l(std::move(ac)));
                    CHECK(4 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0);
                    CHECK(0 == l(ac));
                    CHECK(0 == l(av));
                    CHECK(0 == l(std::move(ac)));
                    CHECK(0 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l1);
                    CHECK(1 == l(ac));
                    CHECK(1 == l(av));
                    CHECK(1 == l(std::move(ac)));
                    //CHECK(1 == l(std::move(v)));
                }
                {
                    auto const l = compose_visitors(l2);
                    //CHECK(2 == l(c));
                    //CHECK(2 == l(v));
                    CHECK(2 == l(std::move(ac)));
                    CHECK(2 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l3);
                    CHECK(3 == l(ac));
                    CHECK(3 == l(av));
                    CHECK(3 == l(std::move(ac)));
                    CHECK(3 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l1);
                    CHECK(0 == l(ac));
                    CHECK(1 == l(av));
                    CHECK(0 == l(std::move(ac)));
                    CHECK(0 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l2);
                    CHECK(0 == l(ac));
                    CHECK(0 == l(av));
                    CHECK(2 == l(std::move(ac)));
                    CHECK(2 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l3);
                    CHECK(0 == l(ac));
                    CHECK(3 == l(av));
                    CHECK(3 == l(std::move(ac)));
                    CHECK(3 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l1, l2);
                    CHECK(1 == l(ac));
                    CHECK(1 == l(av));
                    CHECK(2 == l(std::move(ac)));
                    CHECK(2 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l1, l3);
                    CHECK(1 == l(ac));
                    CHECK(1 == l(av));
                    CHECK(3 == l(std::move(ac)));
                    CHECK(3 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l2, l3);
                    CHECK(3 == l(ac));
                    CHECK(3 == l(av));
                    CHECK(2 == l(std::move(ac)));
                    CHECK(3 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l1, l2);
                    CHECK(0 == l(ac));
                    CHECK(1 == l(av));
                    CHECK(2 == l(std::move(ac)));
                    CHECK(2 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l1, l3);
                    CHECK(0 == l(ac));
                    CHECK(1 == l(av));
                    CHECK(3 == l(std::move(ac)));
                    CHECK(3 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l1, l4);
                    CHECK(0 == l(ac));
                    CHECK(1 == l(av));
                    CHECK(4 == l(std::move(ac)));
                    CHECK(4 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l2, l3);
                    CHECK(0 == l(ac));
                    CHECK(3 == l(av));
                    CHECK(2 == l(std::move(ac)));
                    CHECK(3 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l2, l4);
                    CHECK(0 == l(ac));
                    CHECK(0 == l(av));
                    CHECK(2 == l(std::move(ac)));
                    CHECK(4 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l1, l2, l3);
                    CHECK(1 == l(ac));
                    CHECK(1 == l(av));
                    CHECK(2 == l(std::move(ac)));
                    CHECK(3 == l(std::move(av)));
                }
            }
            {
                auto const l = compose_visitors(l0, l1, l2, l3);
                struct A {};
                using V = variant< RW< A > >;
                V a;
                V const c{};
                {
                    auto const la = compose_visitors([] (A const &) { return -1; }, l);
                    CHECK(-1 == visit(la, c));
                    CHECK(1 == la(a));
                    CHECK(2 == la(std::move(c)));
                    CHECK(3 == la(A{}));
                }
                {
                    struct F { auto operator () (A const &) && { return -11; } };
                    auto lam = compose_visitors(F{}, l);
                    CHECK(0 == lam(c));
                    CHECK(-11 == visit(std::move(lam), c));
                }
            }
        }
        { // function pointers
            using V = variant< decltype(&f), decltype(&g) >;
            V v = g;
            CHECK(v.active< decltype(&f) >());
            CHECK(static_cast< decltype(&f) >(v) == &g);
            CHECK(invoke(v) == 2);
            v = f;
            CHECK(v.active< decltype(&g) >());
            CHECK(static_cast< decltype(&g) >(v) == &f);
            CHECK(invoke(v) == 1);
            auto l = [] { return 323; };
            v = static_cast< decltype(&g) >(l);
            CHECK(invoke(v) == 323);
        }
    }
    { // multivisit mixed visitables
        struct A {};
        struct B {};
        using U = versatile< A, B >;
        using V = variant< A, B >;
        struct
        {
            int operator () (A, A) { return 0; }
            int operator () (A, B) { return 1; }
            int operator () (B, A) { return 2; }
            int operator () (B, B) { return 3; }
        } v;

        A a;
        B b;

        CHECK(multivisit(v, U{a}, V{a}) == 0);
        CHECK(multivisit(v, U{a}, V{b}) == 1);
        CHECK(multivisit(v, U{b}, V{a}) == 2);
        CHECK(multivisit(v, U{b}, V{b}) == 3);

        CHECK(multivisit(v, V{a}, U{a}) == 0);
        CHECK(multivisit(v, V{a}, U{b}) == 1);
        CHECK(multivisit(v, V{b}, U{a}) == 2);
        CHECK(multivisit(v, V{b}, U{b}) == 3);
    }
    { // boost::variant visitation
        struct A {};
        struct B {};
        struct
        {
            int operator () (A, A) { return 0; }
            int operator () (A, B) { return 1; }
            int operator () (B, A) { return 2; }
            int operator () (B, B) { return 3; }
        } v;
        using V = variant_i< ::boost::recursive_wrapper< A >, B >;
        V a{A{}};
        CHECK(a.active< A >());
        CHECK(a.active< ::boost::recursive_wrapper< A > >());
        V b{B{}};
        CHECK(b.active< B >());
        CHECK(multivisit(v, a, a) == 0);
        CHECK(multivisit(v, a, b) == 1);
        CHECK(multivisit(v, b, a) == 2);
        CHECK(multivisit(v, b, b) == 3);
    }
    { // boost::variant visitation
        struct A {};
        struct B {};
        struct
        {
            int operator () (A, A) { return 0; }
            int operator () (A, B) { return 1; }
            int operator () (B, A) { return 2; }
            int operator () (B, B) { return 3; }
        } v;
        using V = variant_c< A, ::boost::recursive_wrapper< B > >;
        V a{A{}};
        CHECK(a.active< A >());
        V b{B{}};
        CHECK(b.active< B >());
        CHECK(b.active< ::boost::recursive_wrapper< B > >());
        CHECK(multivisit(v, a, a) == 0);
        CHECK(multivisit(v, a, b) == 1);
        CHECK(multivisit(v, b, a) == 2);
        CHECK(multivisit(v, b, b) == 3);
    }
    { // handling of the empty
        versatile<> empty;
        using V = versatile< int, double, versatile<> >;
        V v;
        auto l = compose_visitors([] (int) { return 0; }, [] (double) { return 1; }, [] (auto x) { SA(std::is_same< decltype(x), versatile<> >{}); return 2; });
        CHECK(0 == multivisit(l, int{}));
        CHECK(1 == multivisit(l, double{}));
        CHECK(2 == multivisit(l, empty));
        CHECK(0 == multivisit(l, v));
        CHECK(1 == multivisit(l, V{double{}}));
        CHECK(0 == multivisit(l, V{empty})); // default construction equivalent
    }
    { // aggregate wrapper
        struct X {};
        struct Y {};

        struct XY { X x; Y y; XY() = delete; XY(XY const &) = delete; XY(XY &&) = default; XY & operator = (XY &&) = default; };
        XY xy{}; // value-initialization by empty list initializer
        xy.x = {}; xy.y = {}; // accessible
        xy = XY{};
        SA(std::is_constructible< XY, XY >{});
        SA(!std::is_copy_constructible< XY >{});
        SA(std::is_move_constructible< XY >{});
        SA(!std::is_default_constructible< XY >{});

        using WXY = AW< XY >;
        SA(std::is_assignable< WXY &, XY >{});
        SA(std::is_constructible< WXY, XY >{});
        SA(!std::is_copy_constructible< WXY >{}); // mimic
        SA(std::is_move_constructible< WXY >{});
        SA(!std::is_default_constructible< WXY >{});
        WXY wxy{std::move(xy)};
        wxy = XY{};
        wxy = WXY{XY{}};

        using V = variant< WXY >;
        SA(std::is_assignable< V &, XY >{});
        SA(std::is_assignable< V &, WXY >{});
        SA(std::is_constructible< V, XY >{});
        SA(std::is_copy_constructible< V >{}); // lie
        SA(std::is_move_constructible< V >{});
        SA(std::is_default_constructible< V >{});
        V v{in_place, X{}, Y{}};
        CHECK(v.active< XY >());
        CHECK(v.active< WXY >());
        v = XY{};
        v = WXY{XY{}};
        v = V{WXY{XY{}}};
    }
    { // aggregates
        struct X {};
        struct Y {};
        struct XY { X x; Y y = Y{}; };
        using WXY = AW< XY >;
        using V = variant< WXY, X, Y >;
        V v;
        CHECK(v.active< XY >());
        v.replace(X{});
        CHECK(v.active< X >());
        v.replace(Y{});
        CHECK(v.active< Y >());
        v.emplace(X{});
        CHECK(v.active< XY >());
        v.emplace(Y{});
        CHECK(v.active< Y >());
    }
    { // aggregates
        struct X {};
        struct Y {};
        struct XY { X x; Y y; };
        using AXY = AW< XY >;
        using RAXY = RW< AXY >;
        using V = variant< RAXY >;
        V v;
        v = XY{};
        v = AXY{};
        v = RAXY{};
        v = V{};
    }
    { // constexpr
        struct A {};
        struct B {};
        struct C {};
        using V = cvariant< A, B, C >;
        SA(std::is_literal_type< V >{});

        struct visitor3
        {
            constexpr auto operator () (A, int i = 1) const { return 100 + i; }
            constexpr auto operator () (B, int i = 2) const { return 200 + i; }
            constexpr auto operator () (C, int i = 3) const { return 300 + i; }
        };
        SA(std::is_literal_type< visitor3 >{});

        // rrefs
        SA(visit(visitor3{}, V{A{}}) == 101);
        SA(visit(visitor3{}, V{B{}}) == 202);
        SA(visit(visitor3{}, V{C{}}) == 303);

        SA(visit(visitor3{}, V{A{}}, 10) == 110);
        SA(visit(visitor3{}, V{B{}}, 20) == 220);
        SA(visit(visitor3{}, V{C{}}, 30) == 330);

        // lrefs
        constexpr visitor3 visitor3_{};
        constexpr V a{A{}};
        constexpr V b{B{}};
        constexpr V c{C{}};

        SA(visit(visitor3_, a) == 101);
        SA(visit(visitor3_, b) == 202);
        SA(visit(visitor3_, c) == 303);

        SA(visit(visitor3_, a, 11) == 111);
        SA(visit(visitor3_, b, 22) == 222);
        SA(visit(visitor3_, c, 33) == 333);

        struct multivisitor3
        {
            constexpr auto operator () (A, int, A) const { return 111; }
            constexpr auto operator () (A, int, B) const { return 112; }
            constexpr auto operator () (A, int, C) const { return 113; }
            constexpr auto operator () (B, int, A) const { return 121; }
            constexpr auto operator () (B, int, B) const { return 122; }
            constexpr auto operator () (B, int, C) const { return 123; }
            constexpr auto operator () (C, int, A) const { return 131; }
            constexpr auto operator () (C, int, B) const { return 132; }
            constexpr auto operator () (C, int, C) const { return 133; }
            constexpr auto operator () (A, double, A) const { return 211; }
            constexpr auto operator () (A, double, B) const { return 212; }
            constexpr auto operator () (A, double, C) const { return 213; }
            constexpr auto operator () (B, double, A) const { return 221; }
            constexpr auto operator () (B, double, B) const { return 222; }
            constexpr auto operator () (B, double, C) const { return 223; }
            constexpr auto operator () (C, double, A) const { return 231; }
            constexpr auto operator () (C, double, B) const { return 232; }
            constexpr auto operator () (C, double, C) const { return 233; }
        };
        SA(std::is_literal_type< multivisitor3 >{});

        constexpr auto multivisitor3_ = visit(multivisitor3{});
        SA(multivisitor3_(a,   0, a) == 111);
        SA(multivisitor3_(a,   0, b) == 112);
        SA(multivisitor3_(a,   0, c) == 113);
        SA(multivisitor3_(b,   0, a) == 121);
        SA(multivisitor3_(b,   0, b) == 122);
        SA(multivisitor3_(b,   0, c) == 123);
        SA(multivisitor3_(c,   0, a) == 131);
        SA(multivisitor3_(c,   0, b) == 132);
        SA(multivisitor3_(c,   0, c) == 133);
        SA(multivisitor3_(a, 0.0, a) == 211);
        SA(multivisitor3_(a, 0.0, b) == 212);
        SA(multivisitor3_(a, 0.0, c) == 213);
        SA(multivisitor3_(b, 0.0, a) == 221);
        SA(multivisitor3_(b, 0.0, b) == 222);
        SA(multivisitor3_(b, 0.0, c) == 223);
        SA(multivisitor3_(c, 0.0, a) == 231);
        SA(multivisitor3_(c, 0.0, b) == 232);
        SA(multivisitor3_(c, 0.0, c) == 233);
    }
    { // reference_wrapper
        struct A {};
        using RA = std::reference_wrapper< A >;
        using V = variant< RA >;
        A a;
        RA ra{a};
        V v{a};
        V w{ra};
        v = a;
        v = ra;
        v = w;
    }
    {
        CHECK((perferct_forwarding< variant, 2, 6 >{}()));
    }
    { // -ftemplate-depth=40 for 5, 5
        CHECK((hard< ROWS, COLS >()));
    }
    return EXIT_SUCCESS;
}
#endif
