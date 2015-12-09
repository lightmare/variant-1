#pragma once

#include "utility.hpp"

#include <type_traits>
#include <utility>
#include <typeinfo>

#include <cassert>

namespace versatile
{

template< typename type >
struct is_visitable
        : std::false_type
{

};

template< typename type >
constexpr bool is_visitable_v = is_visitable< type >::value;

namespace details
{

template< type_qualifier type_qual, typename visitor, typename visitable >
class dispatcher;

template< type_qualifier type_qual, typename visitor,
          template< typename ...types > class decay_type, typename ...types >
class dispatcher< type_qual, visitor, decay_type< types... > >
{

    using visitable = add_type_qualifier_t< type_qual, decay_type< types... > >;

    template< typename type >
    using qualify_type_t = add_type_qualifier_t< type_qual, unwrap_type_t< type > >;

    using first = qualify_type_t< typename identity< types... >::type >;

    template< typename type, typename ...arguments >
    static
    constexpr
    decltype(std::declval< visitor >()(std::declval< first >(), std::declval< arguments >()...))
    callee(visitor & _visitor, visitable & _visitable, arguments &... _arguments)
    {
        return std::forward< visitor >(_visitor)(static_cast< type >(static_cast< type & >(_visitable)), std::forward< arguments >(_arguments)...);
    }

    template< typename ...arguments >
    using callee_type = decltype(&dispatcher::template callee< first, arguments... >);

    template< typename ...arguments >
    static constexpr callee_type< arguments... > callies_[sizeof...(types)] = {dispatcher::template callee< qualify_type_t< types >, arguments... >...};

public :

    template< typename ...arguments >
    static
    constexpr
    decltype(auto)
    caller(visitor & _visitor, visitable & _visitable, arguments &... _arguments)
    {
        assert(_visitable.which() < sizeof...(types));
        return callies_< arguments... >[sizeof...(types) - 1 - _visitable.which()](_visitor, _visitable, _arguments...);
    }

};

template< type_qualifier type_qual, typename visitor,
          template< typename ...types > class decay_type, typename ...types >
template< typename ...arguments >
constexpr typename dispatcher< type_qual, visitor, decay_type< types... > >::template callee_type< arguments... > dispatcher< type_qual, visitor, decay_type< types... > >::callies_[sizeof...(types)];

}

template< typename visitor, typename visitable, typename ...arguments >
constexpr
decltype(auto)
visit(visitor && _visitor, visitable && _visitable, arguments &&... _arguments)
{
    using decay_type = unwrap_type_t< visitable >;
    static_assert(is_visitable_v< decay_type >, "second argument should be visitable");
    return details::dispatcher< type_qualifier_of< visitable && >, visitor, decay_type >::template caller< arguments... >(_visitor, _visitable, _arguments...);
}

namespace details
{

template< typename supervisitor, typename type, bool = (is_visitable< unwrap_type_t< type > >::value) >
struct subvisitor;

template< typename supervisitor, typename visitable >
struct subvisitor< supervisitor, visitable, true >
{

    supervisitor & supervisitor_;
    visitable & visitable_;

    template< typename ...visited >
    constexpr
    decltype(auto)
    operator () (visited &&... _visited) const
    {
        return visit(std::forward< supervisitor >(supervisitor_), std::forward< visitable >(visitable_), std::forward< visited >(_visited)...);
    }

};

template< typename supervisitor, typename type >
struct subvisitor< supervisitor, type, false >
{

    supervisitor & supervisitor_;
    type & value_;

    template< typename ...visited >
    constexpr
    decltype(auto)
    operator () (visited &&... _visited) const
    {
        return std::forward< supervisitor >(supervisitor_)(std::forward< type >(value_), std::forward< visited >(_visited)...);
    }

};

template< typename ...visitables >
struct visitor_partially_applier;

template<>
struct visitor_partially_applier<>
{

    template< typename visitor >
    static
    constexpr
    decltype(auto)
    call(visitor & _visitor)
    {
        return std::forward< visitor >(_visitor)();
    }

};

template< typename first, typename ...rest >
struct visitor_partially_applier< first, rest... >
{

    template< typename visitor >
    static
    constexpr
    decltype(auto)
    call(visitor & _visitor, first & _first, rest &... _rest)
    {
        return visitor_partially_applier< rest... >::template call< subvisitor< visitor, first > const >({_visitor, _first}, _rest...);
    }

};

}

template< typename visitor, typename ...visitables >
constexpr
decltype(auto)
multivisit(visitor && _visitor, visitables &&... _visitables)
{
    return details::visitor_partially_applier< visitables... >::template call< visitor >(_visitor, _visitables...);
}

namespace details
{

template< typename visitor >
struct delayed_visitor
{

    constexpr
    delayed_visitor(visitor & _visitor) noexcept(std::is_lvalue_reference_v< visitor > || std::is_nothrow_move_constructible_v< visitor >)
        : visitor_(std::forward< visitor >(_visitor))
    { ; }

    template< typename ...types >
    constexpr
    decltype(auto)
    operator () (types &&... _values) &
    {
        return multivisit(visitor_, std::forward< types >(_values)...);
    }

    template< typename ...types >
    constexpr
    decltype(auto)
    operator () (types &&... _values) const &
    {
        return multivisit(std::as_const(visitor_), std::forward< types >(_values)...);
    }

    template< typename ...types >
    constexpr
    decltype(auto)
    operator () (types &&... _values) &&
    {
        return multivisit(std::move(visitor_), std::forward< types >(_values)...);
    }

    template< typename ...types >
    constexpr
    decltype(auto)
    operator () (types &&... _values) const &&
    {
        return multivisit(std::move(std::as_const(visitor_)), std::forward< types >(_values)...);
    }

private :

    visitor visitor_;

};

}

template< typename visitor >
constexpr
details::delayed_visitor< visitor >
visit(visitor && _visitor) noexcept(std::is_lvalue_reference_v< visitor > || std::is_nothrow_move_constructible_v< visitor >)
{
    return _visitor;
}

template< typename visitable, typename ...arguments >
constexpr // lambda is not constexpr yet
decltype(auto)
invoke(visitable && _visitable, arguments &&... _arguments)
{
    return visit([&] (auto && _value) -> decltype(auto)
    {
        return std::forward< decltype(_value) >(_value)(std::forward< arguments >(_arguments)...);
    }, std::forward< visitable >(_visitable));
}

}
