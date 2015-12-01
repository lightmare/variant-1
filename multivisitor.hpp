#pragma once

#include "prologue.hpp"

#include <utility>

namespace test_variant
{

template< typename F, std::size_t ...indices >
struct enumerator
{

    static constexpr std::size_t size_ = sizeof...(indices);
    static constexpr std::size_t count_ = (indices * ...);

    template< typename I, typename C >
    struct decomposer;

    template< std::size_t ...I, std::size_t ...C >
    struct decomposer< std::index_sequence< I... >, std::index_sequence< C... > >
    {

        F & f;

        static constexpr std::size_t indices_[size_] = {indices...};

        static
        constexpr
        std::size_t
        order(std::size_t const i) noexcept
        {
            std::size_t o = 1;
            for (std::size_t n = i + 1; n < size_; ++n) {
                o *= indices_[n];
            }
            return o;
        }

        static constexpr std::size_t orders_[size_] = {order(I)...};

        static
        constexpr
        std::size_t
        digit(std::size_t c, std::size_t const i) noexcept
        {
            for (std::size_t n = 0; n < i; ++n) {
                c = c % orders_[n];
            }
            return c / orders_[i];
        }

        constexpr
        bool
        operator () () const noexcept
        {
            return (call< C >() && ...);
        }

        template< std::size_t c >
        constexpr
        bool
        call() const noexcept
        {
            return f(std::index_sequence< digit(c, I)... >{});
        }

    };

    decomposer< std::make_index_sequence< size_ >, std::make_index_sequence< count_ > > const decomposer_;

    constexpr
    bool
    operator () () const noexcept
    {
        return decomposer_();
    }

};

template< std::size_t ...indices, typename F >
CONSTEXPRF
enumerator< F, indices... >
make_enumerator(F & f) noexcept
{
    SA(0 < sizeof...(indices));
    SA(((0 < indices) && ...));
    return {{f}};
}

using ::versatile::type_qualifier;

template< std::size_t M >
struct pair
{

    type_qualifier qual_ids[1 + M];
    std::size_t type_ids[1 + M];

    CONSTEXPRF
    bool
    operator == (pair const & _rhs) const noexcept
    {
        for (std::size_t i = 0; i <= M; ++i) {
            if (qual_ids[i] != _rhs.qual_ids[i]) {
                return false;
            }
            if (type_ids[i] != _rhs.type_ids[i]) {
                return false;
            }
        }
        return true;
    }

    CONSTEXPRF
    std::size_t
    size() const noexcept
    {
        return (1 + M);
    }

};

using ::versatile::type_qualifier_of;

template< std::size_t M, type_qualifier type_qual = type_qualifier::value >
struct multivisitor
{

    using result_type = pair< M >;

    result_type & result_;

    using return_type = ::versatile::add_type_qualifier_t< type_qual, result_type >;

    static constexpr type_qualifier type_qual_ = type_qual;

    CONSTEXPRF
    std::size_t
    which() const noexcept
    {
        return 1 + M;
    }

    template< typename ...types >
    CONSTEXPRF
    return_type
    operator () (types &&... _values) & noexcept
    {
        //ASSERT (M == sizeof...(types));
        //ASSERT (!(is_visitable_v< types > || ...));
        result_ = {{type_qualifier_of< multivisitor & >, type_qualifier_of< types && >...}, {which(), _values...}};
        return static_cast< return_type >(result_);
    }

    template< typename ...types >
    CONSTEXPRF
    return_type
    operator () (types &&... _values) const & noexcept
    {
        result_ = {{type_qualifier_of< multivisitor const & >, type_qualifier_of< types && >...}, {which(), _values...}};
        return static_cast< return_type >(result_);
    }

    template< typename ...types >
    CONSTEXPRF
    return_type
    operator () (types &&... _values) && noexcept
    {
        result_ = {{type_qualifier_of< multivisitor && >, type_qualifier_of< types && >...}, {which(), _values...}};
        return static_cast< return_type >(result_);
    }

    template< typename ...types >
    CONSTEXPRF
    return_type
    operator () (types &&... _values) const && noexcept
    {
        result_ = {{type_qualifier_of< multivisitor const && >, type_qualifier_of< types && >...}, {which(), _values...}};
        return static_cast< return_type >(result_);
    }

};

static constexpr std::size_t type_qual_begin = static_cast< std::size_t >(type_qualifier_of< void * & >);
static constexpr std::size_t type_qual_end = static_cast< std::size_t >(type_qualifier_of< void * volatile & >);
static constexpr std::size_t ref_count_ = (type_qual_end - type_qual_begin);

template< typename array_type >
struct subscripter
{

    array_type & array_;

    constexpr
    array_type &
    operator () () const noexcept
    {
        return array_;
    }

    template< typename first, typename ...rest >
    constexpr
    decltype(auto)
    operator () (first const & _first, rest const &... _rest) const noexcept
    {
        return operator () (_rest...)[_first];
    }

};

template< typename array_type, typename ...indices >
constexpr
decltype(auto)
subscript(array_type & _array, indices const &... _indices) noexcept
{
    return subscripter< array_type >{_array}(_indices...);
}

template< typename multivisitor, typename variants, typename result_type >
struct fusor
{

    static constexpr std::size_t M = std::extent< variants >::value;

    template< typename = std::make_index_sequence< M > >
    struct fuse;

    template< std::size_t ...i >
    struct fuse< std::index_sequence< i... > >
    {

        multivisitor multivisitor_;
        variants variants_;
        std::size_t counter_;
        result_type result_;

        template< std::size_t m, std::size_t ...v >
        CONSTEXPRF
        bool
        operator () (std::index_sequence< m, v... >) noexcept
        {
            SA(M == sizeof...(v));
            constexpr type_qualifier type_qual_m = static_cast< type_qualifier >(type_qual_begin + m);
            constexpr type_qualifier type_quals_v[sizeof...(v)] = {static_cast< type_qualifier >(type_qual_begin + v)...};
            pair< M > const rhs = {{type_qual_m, type_quals_v[i]...}, {M + 1, variants_[i].which()...}};
            decltype(auto) lhs = ::versatile::multivisit(forward_as< type_qual_m >(multivisitor_),
                                                         forward_as< type_quals_v[i] >(variants_[i])...);
            if (M + 1 != lhs.size()) {
                return false;
            }
            if (type_qualifier_of< decltype(lhs) > != multivisitor_.type_qual_) {
                return false;
            }
            if (!(lhs == rhs)) {
                return false;
            }
            bool & r = subscript(result_, m, v..., (variants_[i].which() - 1)...);
            if (r) {
                return false;
            }
            r = true;
            ++counter_;
            return true;
        }

    };

    fuse<> fuse_;

    constexpr
    auto &
    operator [] (std::size_t const i) noexcept
    {
        return fuse_.variants_[i];
    }

};

template< typename value_type, std::size_t ...extents >
struct multiarray;

template< typename array_type >
struct multiarray< array_type >
{

    using type = array_type;

};

template< typename type, std::size_t first, std::size_t ...rest >
struct multiarray< type, first, rest... >
    : multiarray< type[first], rest... >
{

    using value_type = type;

};

template< typename value_type, std::size_t ...extents >
using multiarray_t = typename multiarray< value_type, extents... >::type;

// variant - variant
// T - type generator
// variant - variant type
// wrapper - wrapper for alternative (bounded) types
// M - multivisitor arity, N - number of alternative (bounded) types
template< template< std::size_t I > class T,
          template< typename ...types > class variant,
          template< typename ...types > class wrapper = ::versatile::identity,
          std::size_t M = 2, std::size_t N = M >
class test_perferct_forwarding
{

    template< type_qualifier type_qual,
              std::size_t ...i, std::size_t ...j >
    CONSTEXPRF
    static
    bool
    run(std::index_sequence< i... >, std::index_sequence< j... >) noexcept
    {
        using multivisitor_type = multivisitor< M, type_qual >;
        typename multivisitor_type::result_type result_{};
        using variant_type = variant< typename wrapper< T< N - j > >::type... >;
        using result_type = multiarray_t< bool, ref_count_, (static_cast< void >(i), ref_count_)..., (static_cast< void >(i), N)... >;
        fusor< multivisitor_type, variant_type [M], result_type > fusor_{{{result_}, {}, 0, {}}};
        auto const enumerator_ = make_enumerator< ref_count_, (static_cast< void >(i), ref_count_)... >(fusor_.fuse_);
        variant_type variants_[N] = {T< N - j >{}...};
        std::size_t indices[M] = {};
        for (;;) {
            ((fusor_[i] = variants_[indices[i]]), ...);
            if (!enumerator_()) {
                return false;
            }
            std::size_t m = 0;
            for (;;) {
                std::size_t & n = indices[m];
                if (++n != N) {
                    break;
                }
                n = 0;
                if (++m == M) {
                    break;
                }
            }
            if (m == M) {
                break;
            }
        }
        constexpr std::size_t count_ = ((static_cast< void >(i), (N * ref_count_)) * ...) * ref_count_; // N ^ M * ref_count_ ^ (M + 1)
        if (fusor_.fuse_.counter_ != count_) {
            return false;
        }
        SA(sizeof(result_type) == count_ * sizeof(bool)); // sizeof(bool) is implementation-defined
        return true;
    }

public :

    CONSTEXPRF
    static
    bool
    run() noexcept
    {
        auto const i = std::make_index_sequence< M >{};
        auto const j = std::make_index_sequence< N >{};
        CHECK (run< type_qualifier::value       >(i, j));
        CHECK (run< type_qualifier::const_value >(i, j));
        CHECK (run< type_qualifier::lref        >(i, j));
        CHECK (run< type_qualifier::rref        >(i, j));
        CHECK (run< type_qualifier::const_lref  >(i, j));
        CHECK (run< type_qualifier::const_rref  >(i, j));
        return true;
    }

};

} // namespace test_variant
