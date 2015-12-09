#pragma once

#include "versatile.hpp"

#include <type_traits>
#include <utility>
#include <experimental/optional>
#include <memory>
#include <typeinfo>

#include <cstddef>
#include <cassert>

namespace versatile
{

template< typename ...types >
class variant
{

    using versatile = versatile< types... >;
    using storage_type = std::unique_ptr< versatile >; // `All problems in computer science can be solved by another level of indirection, except for the problem of too many layers of indirection.`

    storage_type storage_;

public :

    using default_index = typename versatile::default_index;
    template< typename ...arguments >
    using index_of_constructible = typename versatile::template index_of_constructible< arguments... >;

    std::size_t
    which() const noexcept
    {
        return storage_->which();
    }

    template< typename type >
    using index_at_t = typename versatile::template index_at_t< type >;

    template< typename type >
    bool
    active() const noexcept
    {
        return storage_->template active< type >();
    }

private :

    struct constructor
    {

        template< typename type >
        storage_type
        operator () (type && _value) const
        {
            return std::make_unique< versatile >(std::forward< type >(_value));
        }

    };

public :

    variant(variant & _rhs)
        : storage_(visit(constructor{}, *_rhs.storage_))
    { ; }

    variant(variant const & _rhs)
        : storage_(visit(constructor{}, *_rhs.storage_))
    { ; }

    variant(variant && _rhs)
        : storage_(visit(constructor{}, std::move(*_rhs.storage_)))
    { ; }

    variant(variant const && _rhs)
        : storage_(visit(constructor{}, std::move(*_rhs.storage_)))
    { ; }

    variant()
        : storage_(std::make_unique< versatile >())
    { ; }

    template< typename type, typename index = index_at_t< type > >
    variant(type && _value)
        : storage_(std::make_unique< versatile >(std::forward< type >(_value)))
    { ; }

    template< std::size_t i, typename ...arguments >
    explicit
    variant(in_place_t (&)(index_t< i >), arguments &&... _arguments)
        : storage_(std::make_unique< versatile >(in_place< i >, std::forward< arguments >(_arguments)...))
    { ; }

    template< typename type, typename ...arguments >
    explicit
    variant(in_place_t (&)(identity< type >), arguments &&... _arguments)
        : storage_(std::make_unique< versatile >(in_place< type >, std::forward< arguments >(_arguments)...))
    { ; }

    void
    swap(variant & _other) noexcept
    {
        return storage_.swap(_other.storage_);
    }

    template< std::size_t i, typename ...arguments >
    void
    emplace(arguments &&... _arguments)
    {
        return variant{in_place< i >, std::forward< arguments >(_arguments)...}.swap(*this);
    }

    template< typename type, typename ...arguments >
    void
    emplace(arguments &&... _arguments)
    {
        return variant{in_place< type >, std::forward< arguments >(_arguments)...}.swap(*this);
    }

private :

    struct assigner
    {

        versatile & destination_;

        template< typename type >
        void
        operator () (type && _value) const
        {
            static_cast< unwrap_type_t< type > & >(destination_) = std::forward< type >(_value);
        }

    };

public :

    template< typename type >
    variant &
    assign(type && _rhs)
    {
        assert(&_rhs != this);
        if (which() == _rhs.which()) {
            visit(assigner{*storage_},  std::forward< type >(_rhs));
        } else {
            variant(std::forward< type >(_rhs)).swap(*this);
        }
        return *this;
    }

    variant &
    operator = (variant & _rhs) &
    {
        return assign(_rhs);
    }

    variant &
    operator = (variant const & _rhs) &
    {
        return assign(_rhs);
    }

    variant &
    operator = (variant && _rhs) &
    {
        return assign(std::move(_rhs));
    }

    variant &
    operator = (variant const && _rhs) &
    {
        return assign(std::move(_rhs));
    }

    template< typename type, typename index = index_at_t< type > >
    variant &
    operator = (type && _value) &
    {
        using decay_type = unwrap_type_t< type >;
        if (active< decay_type >()) {
            static_cast< decay_type & >(*storage_) = std::forward< type >(_value);
        } else {
            variant(std::forward< type >(_value)).swap(*this);
        }
        return *this;
    }

    template< typename type, typename index = index_at_t< type > >
    explicit
    operator type & ()
    {
        if (!active< type >()) {
            throw std::bad_cast{};
        }
        return static_cast< type & >(*storage_);
    }

    template< typename type, typename index = index_at_t< type > >
    explicit
    operator type const & () const
    {
        if (!active< type >()) {
            throw std::bad_cast{};
        }
        return static_cast< type const & >(*storage_);
    }

};

template<>
class variant<>
{

};

template< typename first, typename ...rest >
constexpr
void
swap(variant< first, rest... > & _lhs, variant< first, rest... > & _rhs) noexcept
{
    _lhs.swap(_rhs);
}

template< typename first, typename ...rest >
struct is_visitable< variant< first, rest... > >
        : std::true_type
{

};

}
