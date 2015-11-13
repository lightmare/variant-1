#pragma once

#include "wrappers.hpp"

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

namespace details
{

template< typename visitor, typename visitable, typename decay_type = unwrap_type_t< visitable > >
struct dispatcher;

template< typename visitor, typename visitable,
          template< typename ...types > class decay_type, typename ...types >
struct dispatcher< visitor, visitable, decay_type< types... > >
{

    template< typename ...arguments >
    static
    constexpr
    decltype(auto)
    caller(visitor & _visitor, visitable & _visitable, arguments &... _arguments)
    {
        assert(!(sizeof...(types) < _visitable.which()));
        return callies_< arguments... >[sizeof...(types) - _visitable.which()](_visitor, _visitable, _arguments...);
    }

private :

    template< typename type, typename ...arguments >
    static
    constexpr
    decltype(auto)
    callee(visitor & _visitor, visitable & _visitable, arguments &... _arguments)
    {
        return std::forward< visitor >(_visitor)(static_cast< type >(static_cast< type & >(_visitable)), std::forward< arguments >(_arguments)...);
    }

    static constexpr type_qualifier type_qualifier_ = type_qualifier_of< visitable && >;

    template< typename type >
    using qualify_type_t = add_qualifier_t< type_qualifier_, unwrap_type_t< type > >;

    template< typename ...arguments >
    using callee_type = decltype(&dispatcher::template callee< qualify_type_t< typename identity< types... >::type >, arguments... >);

    template< typename ...arguments >
    static constexpr callee_type< arguments... > callies_[sizeof...(types)] = {dispatcher::template callee< qualify_type_t< types >, arguments... >...};

};

template< typename visitor, typename visitable,
          template< typename ...types > class decay_type, typename ...types >
template< typename ...arguments >
constexpr typename dispatcher< visitor, visitable, decay_type< types... > >::template callee_type< arguments... > dispatcher< visitor, visitable, decay_type< types... > >::callies_[sizeof...(types)];

}

template< typename visitor, typename visitable, typename ...arguments >
constexpr
decltype(auto)
visit(visitor && _visitor, visitable && _visitable, arguments &&... _arguments)
{
    return details::template dispatcher< visitor, visitable >::template caller< arguments... >(_visitor, _visitable, _arguments...);
}

namespace details
{

template< typename supervisitor, typename type, bool = (is_visitable< unwrap_type_t< type > >{}) >
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
    constexpr
    decltype(auto)
    operator () (visitor && _visitor) const
    {
        return std::forward< visitor >(_visitor)();
    }

};

template< typename first, typename ...rest >
struct visitor_partially_applier< first, rest... >
{

    template< typename visitor >
    constexpr
    decltype(auto)
    operator () (visitor && _visitor, first & _first, rest &... _rest) const
    {
        return visitor_partially_applier< rest... >{}(subvisitor< visitor, first >{_visitor, _first}, _rest...);
    }

};

}

template< typename visitor, typename ...visitables >
constexpr
decltype(auto)
multivisit(visitor && _visitor, visitables &&... _visitables)
{
    return details::visitor_partially_applier< visitables... >{}(std::forward< visitor >(_visitor), _visitables...);
}

namespace details
{

template< typename visitor >
struct delayed_visitor
{

    constexpr
    delayed_visitor(visitor & _visitor) noexcept(std::is_lvalue_reference< visitor >{} || std::is_nothrow_move_constructible< visitor >{})
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
        return multivisit(visitor_, std::forward< types >(_values)...);
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
        return multivisit(std::move(visitor_), std::forward< types >(_values)...);
    }

private :

    visitor visitor_;

};

}

template< typename visitor >
constexpr
details::delayed_visitor< visitor >
visit(visitor && _visitor) noexcept(std::is_lvalue_reference< visitor >{} || std::is_nothrow_move_constructible< visitor >{})
{
    return _visitor;
}

namespace details
{

template< typename visitor, typename ...visitors >
struct composite_visitor
        : unwrap_type_t< visitor >
        , composite_visitor< visitors... >
{

    using head = unwrap_type_t< visitor >;
    using tail = composite_visitor< visitors... >;

    using head::operator ();
    using tail::operator ();

    constexpr
    composite_visitor(visitor & _visitor, visitors &... _visitors) noexcept(std::is_nothrow_constructible< head, visitor >{} && std::is_nothrow_constructible< tail, visitors &... >{})
        : head(std::forward< visitor >(_visitor))
        , tail{_visitors...}
    { ; }

};

template< typename visitor >
struct composite_visitor< visitor >
        : unwrap_type_t< visitor >
{

    using base = unwrap_type_t< visitor >;

    using base::operator ();

    constexpr
    composite_visitor(visitor & _visitor) noexcept(std::is_nothrow_constructible< base, visitor >{})
        : base(std::forward< visitor >(_visitor))
    { ; }

};

}

template< typename visitor, typename ...visitors >
constexpr
details::composite_visitor< visitor, visitors... >
compose_visitors(visitor && _visitor, visitors &&... _visitors)
{
    return {_visitor, _visitors...};
}

template< typename visitable, typename ...arguments >
decltype(auto)
call(visitable && _visitable, arguments &&... _arguments)
{
    return visit([&] (auto && _value) -> decltype(auto)
    {
        return std::forward< decltype(_value) >(_value)(std::forward< arguments >(_arguments)...);
    }, std::forward< visitable >(_visitable));
}

}
