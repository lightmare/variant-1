#include "type_traits.hpp"
#include "recursive_wrapper.hpp"
#include "variant.hpp"
#include "visitation.hpp"

#include <string>
#include <array>
#include <utility>
#include <tuple>
#ifdef _DEBUG
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
    using variant::apply_visitor;
    return (std::array< std::size_t, sizeof...(N) >{(N % sizeof...(M))...} == apply_visitor(visitor{}, variant::variant< T< M >... >{T< (N % sizeof...(M)) >{}}...));
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

int
main()
{
    {
        using namespace variant;
        {
            using V = variant< int, float, double, long double >;
            assert(V{0}.which() == 4);
            assert(V{1.0f}.which() == 3);
            assert(V{2.0}.which() == 2);
            assert(V{3.0L}.which() == 1);
        }
        {
            struct A;
            struct B {};
            using V = variant< recursive_wrapper< A >, B >;
            struct A {};
            assert(V{A{}}.which() == 2);
        }
    }
    {
        assert((test< ROWS, COLS >())); // 9 seconds (Release build) for COLS=5 ROWS=5 on Intel(R) Xeon(R) CPU E5-1650 0 @ 3.20GHz
    }
    return EXIT_SUCCESS;
}
