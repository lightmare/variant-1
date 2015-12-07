#pragma once

#include <versatile.hpp>

namespace versatile
{ // little extension (candidates to include in library)

template< type_qualifier type_qual, typename type >
constexpr
decltype(auto)
forward_as(type && _value) noexcept
{
    return static_cast< add_type_qualifier_t< type_qual, std::decay_t< type > > >(_value);
}

template< typename type, typename V >
constexpr
bool
is_active(V const & v) noexcept
{
    return v.template active< type >();
}

template< typename type, typename V >
constexpr
type const &
get(V const & v) noexcept
{
    return static_cast< type const & >(v);
}

template< typename type, typename V >
constexpr
type &
get(V & v) noexcept
{
    return static_cast< type & >(v);
}

template< typename type, typename V >
constexpr
type const &&
get(V const && v) noexcept
{
    return static_cast< type const && >(static_cast< type const & >(v));
}

template< typename type, typename V >
constexpr
type &&
get(V && v) noexcept
{
    return static_cast< type && >(static_cast< type & >(v));
}

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