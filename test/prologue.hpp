#pragma once

#include <type_traits>
#include <utility>

#if defined(DEBUG) || defined(_DEBUG)
#include <iostream>
#include <iomanip>
#endif

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

#define STR(S) #S
// double expansion of macro argument
#define STRN(N) STR(N)
#define LOCATION "file '" __FILE__ "', line: " STRN(__LINE__)
#define SA(...) static_assert((__VA_ARGS__), LOCATION)
#define CHECK(...) { assert((__VA_ARGS__)); }

#if 1
#define ASSERT(...) { SA(__VA_ARGS__); }
#define CONSTEXPR constexpr
#define CONSTEXPRF constexpr
#define DESTRUCTOR = default;
//#define CBRA { struct _ { static constexpr bool call() noexcept {
//#define CKET return true; } }; SA(_::call()); }
#else
#define ASSERT(...) { assert((__VA_ARGS__)); }
#define CONSTEXPR const
#define CONSTEXPRF
#define DESTRUCTOR { ; }
//#define CBRA {
//#define CKET }
#endif


namespace test
{

template< std::size_t I = 0 >
struct literal_type
{

    std::size_t state = I;

};

SA(!std::is_trivially_default_constructible_v< literal_type<> >);
SA(std::is_default_constructible_v< literal_type<> >);
SA(std::is_literal_type_v< literal_type<> >);
SA(std::is_trivially_copyable_v< literal_type<> >);

template< std::size_t I = 0 >
struct common_type
{

    std::size_t state = I;

    common_type() { ; }
    common_type(common_type const & c) : state(c.state) { ; }
    common_type(common_type & c) : state(c.state) { ; }
    common_type(common_type && c) : state(c.state) { ; }
    common_type & operator = (common_type const & c) { state = c.state; return *this; }
    common_type & operator = (common_type & c) { state = c.state; return *this; }
    common_type & operator = (common_type && c) { state = c.state; return *this; }
    ~common_type() { state = ~std::size_t{}; }

};

SA(!std::is_literal_type_v< common_type<> >);
SA(!std::is_trivially_default_constructible_v< common_type<> >);
SA(!std::is_trivially_destructible_v< common_type<> >);
SA(!std::is_trivially_copy_constructible_v< common_type<> >);
SA(!std::is_trivially_move_constructible_v< common_type<> >);
SA(!std::is_trivially_copy_assignable_v< common_type<> >);
SA(!std::is_trivially_move_assignable_v< common_type<> >);
SA(std::is_default_constructible_v< common_type<> >);
SA(std::is_destructible_v< common_type<> >);
SA(std::is_copy_constructible_v< common_type<> >);
SA(std::is_move_constructible_v< common_type<> >);
SA(std::is_copy_assignable_v< common_type<> >);
SA(std::is_move_assignable_v< common_type<> >);

template< typename type >
constexpr bool is_vcopy_constructible_v = std::is_constructible_v< type, type & >;

template< typename type >
constexpr bool is_cmove_constructible_v = std::is_constructible_v< type, type const && >;

template< typename type >
constexpr bool is_vcopy_assignable_v = std::is_assignable_v< type &, type & >;

template< typename type >
constexpr bool is_cmove_assignable_v = std::is_assignable_v< type &, type const && >;

template< typename type >
constexpr bool is_trivially_vcopy_constructible_v = std::is_trivially_constructible_v< type, type & >;

template< typename type >
constexpr bool is_trivially_cmove_constructible_v = std::is_trivially_constructible_v< type, type const && >;

template< typename type >
constexpr bool is_trivially_vcopy_assignable_v = std::is_trivially_assignable_v< type &, type & >;

template< typename type >
constexpr bool is_trivially_cmove_assignable_v = std::is_trivially_assignable_v< type &, type const && >;

template< typename from, typename to >
struct is_explicitly_convertible // akrzemi1's answer http://stackoverflow.com/a/16894048/1430927
        : std::bool_constant< (std::is_constructible_v< to, from > && !std::is_convertible< from, to >::value) >
{

};

template< typename from, typename to >
constexpr bool is_explicitly_convertible_v = is_explicitly_convertible< from, to >::value;

} // namespace test
