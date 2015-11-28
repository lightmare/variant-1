#pragma once

#include "visit.hpp"

#include <type_traits>
#include <utility>
#include <experimental/optional>

#include <cstddef>
#include <cassert>

namespace versatile
{

template< bool is_trivially_destructible, typename ...types >
struct destructor_dispatcher;

template< bool is_trivially_destructible >
struct destructor_dispatcher< is_trivially_destructible >
{

    constexpr
    void
    destructor(std::size_t const) const noexcept
    { ; }

};

template< typename first, typename ...rest >
struct destructor_dispatcher< true, first, rest... >
{

    using head = first;
    using tail = destructor_dispatcher< true, rest... >;

    union
    {

        head head_;
        tail tail_;

    };

    constexpr
    destructor_dispatcher() = default;

    template< typename ...arguments >
    constexpr
    destructor_dispatcher(index_t< (sizeof...(rest) + 1) >, arguments &&... _arguments) noexcept(noexcept(::new (nullptr) head(std::declval< arguments >()...)))
        : head_(std::forward< arguments >(_arguments)...)
    { ; }

    template< typename ...arguments >
    constexpr
    destructor_dispatcher(arguments &&... _arguments) noexcept(noexcept(::new (nullptr) tail(std::declval< arguments >()...)))
        : tail_(std::forward< arguments >(_arguments)...)
    { ; }

    using this_type = unwrap_type_t< first >;

    explicit
    constexpr
    operator this_type const & () const noexcept
    {
        return static_cast< this_type const & >(head_);
    }

    explicit
    constexpr
    operator this_type & () noexcept
    {
        return static_cast< this_type & >(head_);
    }

    template< typename type >
    explicit
    constexpr
    operator type const & () const noexcept
    {
        return static_cast< type const & >(tail_);
    }

    template< typename type >
    explicit
    constexpr
    operator type & () noexcept
    {
        return static_cast< type & >(tail_);
    }

};

template< typename first, typename ...rest >
struct destructor_dispatcher< false, first, rest... >
{

    using head = first;
    //using tail = destructor_dispatcher< (std::is_trivially_destructible_v< rest > && ...), rest... >; // redundant overengeneering
    using tail = destructor_dispatcher< false, rest... >;

    union
    {

        head head_;
        tail tail_;

    };

    ~destructor_dispatcher() noexcept
    {
        tail_.~tail(); // trivial tail is not specially processed, because whole versatile type can't get an advantage from it
    }

    void
    free(std::size_t const _which) const noexcept(noexcept(destructor(_which)))
    {
        if (_which == std::size_t{}) { // trivially constructible case
            head_.~head();
        } else {
            destructor(_which);
        }
    }

    void
    destructor(std::size_t const _which) const noexcept(noexcept(head_.~head()) && noexcept(tail_.destructor(_which)))
    {
        if (_which == (sizeof...(rest) + 1)) {
            head_.~head();
        } else {
            tail_.destructor(_which);
        }
    }

    constexpr
    destructor_dispatcher() = default;

    template< typename ...arguments >
    constexpr
    destructor_dispatcher(index_t< (sizeof...(rest) + 1) >, arguments &&... _arguments) noexcept(noexcept(::new (nullptr) head(std::declval< arguments >()...)))
        : head_(std::forward< arguments >(_arguments)...)
    { ; }

    template< typename ...arguments >
    constexpr
    destructor_dispatcher(arguments &&... _arguments) noexcept(noexcept(::new (nullptr) tail(std::declval< arguments >()...)))
        : tail_(std::forward< arguments >(_arguments)...)
    { ; }

    using this_type = unwrap_type_t< first >;

    explicit
    constexpr
    operator this_type const & () const noexcept
    {
        return static_cast< this_type const & >(head_);
    }

    explicit
    constexpr
    operator this_type & () noexcept
    {
        return static_cast< this_type & >(head_);
    }

    template< typename type >
    explicit
    constexpr
    operator type const & () const noexcept
    {
        return static_cast< type const & >(tail_);
    }

    template< typename type >
    explicit
    constexpr
    operator type & () noexcept
    {
        return static_cast< type & >(tail_);
    }

};

template< bool is_trivially_destructible, bool is_trivially_constructible, typename ...types >
struct dispatcher;

template< typename ...types >
struct dispatcher< true, true, types... >
{

    using storage = destructor_dispatcher< true, types... >;

    std::size_t which_;
    storage storage_;

    constexpr
    dispatcher() = default;

    template< typename index, typename ...arguments >
    constexpr
    dispatcher(index, arguments &&... _arguments) noexcept(noexcept(::new (nullptr) storage(index{}, std::forward< arguments >(_arguments)...)))
        : which_{index::value}
        , storage_(index{}, std::forward< arguments >(_arguments)...)
    { ; }

    template< typename type >
    explicit
    constexpr
    operator type const & () const noexcept
    {
        return static_cast< type const & >(storage_);
    }

    template< typename type >
    explicit
    constexpr
    operator type & () noexcept
    {
        return static_cast< type & >(storage_);
    }

};

template< typename ...types >
struct dispatcher< false, true, types... >
{

    using storage = destructor_dispatcher< false, types... >;

    std::size_t which_;
    storage storage_;

    ~dispatcher() noexcept(noexcept(storage_.destructor(which_)))
    {
        storage_.free(which_);
    }

    constexpr
    dispatcher() = default;

    template< typename index, typename ...arguments >
    constexpr
    dispatcher(index, arguments &&... _arguments) noexcept(noexcept(::new (nullptr) storage(index{}, std::forward< arguments >(_arguments)...)))
        : which_{index::value}
        , storage_(index{}, std::forward< arguments >(_arguments)...)
    { ; }

    template< typename type >
    explicit
    constexpr
    operator type const & () const noexcept
    {
        return static_cast< type const & >(storage_);
    }

    template< typename type >
    explicit
    constexpr
    operator type & () noexcept
    {
        return static_cast< type & >(storage_);
    }

};

template< typename ...types >
struct dispatcher< true, false, types... >
{

    using storage = destructor_dispatcher< true, types... >;

    std::size_t which_;
    storage storage_;

    using default_index = index_of_default_constructible< types..., void >;

    constexpr
    dispatcher() noexcept(noexcept(::new (nullptr) dispatcher(typename default_index::type{})))
        : dispatcher(typename default_index::type{})
    { ; }

    template< typename index, typename ...arguments >
    constexpr
    dispatcher(index, arguments &&... _arguments) noexcept(noexcept(::new (nullptr) storage(index{}, std::forward< arguments >(_arguments)...)))
        : which_{index::value}
        , storage_(index{}, std::forward< arguments >(_arguments)...)
    { ; }

    template< typename type >
    explicit
    constexpr
    operator type const & () const noexcept
    {
        return static_cast< type const & >(storage_);
    }

    template< typename type >
    explicit
    constexpr
    operator type & () noexcept
    {
        return static_cast< type & >(storage_);
    }

};

template< typename ...types >
struct dispatcher< false, false, types... >
{

    using storage = destructor_dispatcher< false, types... >;

    std::size_t which_;
    storage storage_;

    ~dispatcher() noexcept(noexcept(storage_.destructor(which_)))
    {
        storage_.free(which_);
    }

    using default_index = index_of_default_constructible< types..., void >;

    constexpr
    dispatcher() noexcept(noexcept(::new (nullptr) dispatcher(typename default_index::type{})))
        : dispatcher(typename default_index::type{})
    { ; }

    template< typename index, typename ...arguments >
    constexpr
    dispatcher(index, arguments &&... _arguments) noexcept(noexcept(::new (nullptr) storage(index{}, std::forward< arguments >(_arguments)...)))
        : which_{index::value}
        , storage_(index{}, std::forward< arguments >(_arguments)...)
    { ; }

    template< typename type >
    explicit
    constexpr
    operator type const & () const noexcept
    {
        return static_cast< type const & >(storage_);
    }

    template< typename type >
    explicit
    constexpr
    operator type & () noexcept
    {
        return static_cast< type & >(storage_);
    }

};

template< typename ...types >
using dispatcher_t = dispatcher< (std::is_trivially_destructible_v< types > && ...), (std::is_trivially_default_constructible_v< types > && ...), types... >;

template< bool is_default_constructible >
struct enable_default_constructor;

template<>
struct enable_default_constructor< true >
{

    constexpr
    enable_default_constructor() = default;

    constexpr
    enable_default_constructor(void *) noexcept
    { ; }

};

template<>
struct enable_default_constructor< false >
{

    constexpr
    enable_default_constructor() = delete;

    constexpr
    enable_default_constructor(void *) noexcept
    { ; }

};

template< typename ...types >
using enable_default_constructor_t = enable_default_constructor< (std::is_default_constructible_v< types > || ...) >;

template< typename ...types >
class versatile
        : enable_default_constructor_t< types... >
{

    using enabler = enable_default_constructor_t< types... >;
    using storage = dispatcher_t< types... >;

    storage storage_;

public :

    using default_index = index_of_default_constructible< types..., void >;

    constexpr
    std::size_t
    which() const noexcept
    {
        if (storage_.which_ == std::size_t{}) { // is trivially default constructed?
            return sizeof...(types); // if so, then it always point to the first (leftmost) alternative type
        }
        return storage_.which_;
    }

    template< typename type >
    using index_at_t = index_at_t< unwrap_type_t< type >, unwrap_type_t< types >..., void >;

    template< typename type >
    constexpr
    bool
    active() const noexcept
    {
        return (which() == index_at_t< type >::value);
    }

    constexpr
    versatile() = default;

    template< typename type, typename index = index_at_t< type > >
    constexpr
    versatile(type && _value) noexcept(noexcept(::new (nullptr) storage(index{}, std::forward< type >(_value))))
        : enabler({})
        , storage_(index{}, std::forward< type >(_value))
    { ; }

    template< std::size_t i, typename ...arguments >
    constexpr
    versatile(index< i >, arguments &&... _arguments) noexcept(noexcept(::new (nullptr) storage(index_t< i >{}, std::forward< arguments >(_arguments)...)))
        : enabler({})
        , storage_(index_t< i >{}, std::forward< arguments >(_arguments)...)
    { ; }

    template< typename type,
              typename index = index_at_t< type > >
    explicit
    constexpr
    operator type const & () const noexcept
    {
        return static_cast< type const & >(storage_);
    }

    template< typename type,
              typename index = index_at_t< type > >
    explicit
    constexpr
    operator type & () noexcept
    {
        return static_cast< type & >(storage_);
    }

    template< typename type,
              typename index = index_at_t< type > >
    constexpr
    versatile &
    operator = (type && _value) noexcept
    {
        return (*this = versatile(std::forward< type >(_value))); // http://stackoverflow.com/questions/33936295/
    }

    template< std::size_t i = sizeof...(types), typename ...arguments >
    constexpr
    void
    emplace(arguments &&... _arguments) noexcept
    {
        *this = versatile(index< i >{}, std::forward< arguments >(_arguments)...);
    }

};

template< typename first, typename ...rest >
struct is_visitable< versatile< first, rest... > >
        : std::true_type
{

};

}
