#if 1
#include <versatile/wrappers.hpp>

#include <utility>

namespace versatile
{

template< bool is_trivially_destructible, typename ...types >
struct constructor;

template< bool is_trivially_destructible >
struct constructor< is_trivially_destructible >
{

    static_assert(is_trivially_destructible);

    constexpr
    void
    destructor(std::size_t & _which) const noexcept
    {
        _which = 0;
    }

};

template< typename first, typename ...rest >
struct constructor< true, first, rest... >
{

    static_assert((__has_trivial_destructor(rest) && ...));

    using head = first;
    using tail = constructor< true, rest... >;

    union
    {

        head head_;
        tail tail_;

    };

    constexpr
    constructor() = default;

    template< typename ...arguments >
    constexpr
    constructor(index_t< sizeof...(rest) + 1 >,
                arguments &&... _arguments) noexcept(__is_nothrow_constructible(head, arguments...))
        : head_(std::forward< arguments >(_arguments)...)
    { ; }

    template< typename ...arguments >
    constexpr
    constructor(arguments &&... _arguments) noexcept(__is_nothrow_constructible(tail, arguments...))
        : tail_(std::forward< arguments >(_arguments)...)
    { ; }

    constexpr
    void
    destructor(std::size_t & _which) const noexcept
    {
        _which = 0;
    }

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
struct constructor< false, first, rest... >
{

    using head = first;
    using tail = constructor< (__has_trivial_destructor(rest) && ...), rest... >;

    union
    {

        head head_;
        tail tail_;

    };

    ~constructor() noexcept
    {
        tail_.~tail();
    }

    constexpr
    constructor() = default;

    template< typename ...arguments >
    constexpr
    constructor(index_t< sizeof...(rest) + 1 >,
                arguments &&... _arguments) noexcept(__is_nothrow_constructible(head, arguments...))
        : head_(std::forward< arguments >(_arguments)...)
    { ; }

    template< typename ...arguments >
    constexpr
    constructor(arguments &&... _arguments) noexcept(__is_nothrow_constructible(tail, arguments...))
        : tail_(std::forward< arguments >(_arguments)...)
    { ; }

    constexpr
    void
    destructor(std::size_t & _which) const noexcept(noexcept(head_.~head()) && noexcept(tail_.destructor(_which)))
    {
        if (_which == sizeof...(rest) + 1) {
            head_.~head();
        } else {
            tail_.destructor(_which);
        }
    }

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
struct destructor;

template< bool is_trivially_destructible, bool is_trivially_constructible >
struct destructor< is_trivially_destructible, is_trivially_constructible >
{

    static_assert(is_trivially_destructible);
    static_assert(is_trivially_constructible);

};

template< typename first, typename ...rest >
struct destructor< true, true, first, rest... >
{

    using storage = constructor< true, first, rest... >;

    std::size_t which_;
    storage storage_;

    constexpr
    destructor() = default;

    template< typename index,
              typename ...arguments >
    constexpr
    destructor(index,
               arguments &&... _arguments) noexcept(__is_nothrow_constructible(storage, index, arguments...))
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

template< typename first, typename ...rest >
struct destructor< false, true, first, rest... >
{

    using storage = constructor< false, first, rest... >;

    std::size_t which_;
    storage storage_;

    ~destructor() noexcept(noexcept(storage_.destructor(which_)))
    {
        storage_.destructor(which_);
    }

    constexpr
    destructor() = default;

    template< typename index,
              typename ...arguments >
    constexpr
    destructor(index,
               arguments &&... _arguments) noexcept(__is_nothrow_constructible(storage, index, arguments...))
        : which_{index::value}
        , storage_(index{},
                   std::forward< arguments >(_arguments)...)
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

template< typename first, typename ...rest >
struct destructor< true, false, first, rest... >
{

    using storage = constructor< true, first, rest... >;

    std::size_t which_;
    storage storage_;

    constexpr
    destructor() noexcept(__is_nothrow_constructible(destructor, index_of_default_constructible_t< first, rest..., void >))
        : destructor(index_of_default_constructible_t< first, rest..., void >{})
    { ; }

    template< typename index,
              typename ...arguments >
    constexpr
    destructor(index,
               arguments &&... _arguments) noexcept(__is_nothrow_constructible(storage, index, arguments...))
        : which_{index::value}
        , storage_(index{},
                   std::forward< arguments >(_arguments)...)
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

template< typename first, typename ...rest >
struct destructor< false, false, first, rest... >
{

    using storage = constructor< false, first, rest... >;

    std::size_t which_;
    storage storage_;

    ~destructor() noexcept(noexcept(storage_.destructor(which_)))
    {
        storage_.destructor(which_);
    }

    constexpr
    destructor() noexcept(__is_nothrow_constructible(destructor, index_of_default_constructible_t< first, rest..., void >))
        : destructor(index_of_default_constructible_t< first, rest..., void >{})
    { ; }

    template< typename index,
              typename ...arguments >
    constexpr
    destructor(index,
               arguments &&... _arguments) noexcept(__is_nothrow_constructible(storage, index, arguments...))
        : which_{index::value}
        , storage_(index{},
                   std::forward< arguments >(_arguments)...)
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
class variant
{

    using storage = destructor< (__has_trivial_destructor(types) && ...), (__has_trivial_constructor(types) && ...), types... >;

    storage storage_;

public :

    constexpr
    std::size_t
    which() const noexcept
    {
        if (storage_.which_ == 0) {
            return sizeof...(types);
        }
        return storage_.which_;
    }

    template< typename type >
    using index = index_at< unwrap_type_t< type >, unwrap_type_t< types >..., void >;

    template< typename type >
    constexpr
    bool
    active() const noexcept
    {
        return (which() == index< type >::value);
    }

    constexpr
    variant() = default;

    template< typename type,
              typename index = index_at_t< unwrap_type_t< type >, unwrap_type_t< types >..., void > >
    constexpr
    variant(type && _value) noexcept(__is_nothrow_constructible(storage, index, type))
        : storage_(index{},
                   std::forward< type >(_value))
    { ; }

    template< typename ...arguments,
              typename index = get_index_t< __is_constructible(types, arguments...)..., false > >
    explicit // crucial thing: make the c-tor non-viable for conversion during this-assignment
    constexpr
    variant(arguments &&... _arguments) noexcept(__is_nothrow_constructible(storage, index, arguments...))
        : storage_(index{},
                   std::forward< arguments >(_arguments)...)
    { ; }

    template< typename type,
              typename index = index_at_t< unwrap_type_t< type >, unwrap_type_t< types >..., void > >
    explicit
    constexpr
    operator type const & () const noexcept
    {
        return static_cast< type const & >(storage_);
    }

    template< typename type,
              typename index = index_at_t< unwrap_type_t< type >, unwrap_type_t< types >..., void > >
    explicit
    constexpr
    operator type & () noexcept
    {
        return static_cast< type & >(storage_);
    }

    template< typename type,
              typename = get_index_t< (__has_trivial_assign(type) && ... && __has_trivial_assign(types)) >, // enable_if_t
              typename index = index_at_t< unwrap_type_t< type >, unwrap_type_t< types >..., void > >
    constexpr
    variant &
    operator = (type && _value) noexcept // trivial per se
    {
        return *this = variant(std::forward< type >(_value)); // http://stackoverflow.com/questions/33936295/
    }


};

} // namespace variant

#include <new>
#include <tuple>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

#if 1
#define ASSERT(...) { static_assert(__VA_ARGS__); }
#define CONSTEXPR constexpr
#else
#define ASSERT(...) { assert((__VA_ARGS__)); }
#define CONSTEXPR const
#endif
#define EQUIV(lhs, rhs) __extension__ ({ auto && l = (lhs); auto && r = (rhs); (!((r < l) || (l < r))); })
#define CHECK(...) { if (!(__VA_ARGS__)) return false; }

namespace versatile
{
namespace test
{

#define ENABLE(E, d, dc, cc, cc_, mc, ca, ca_, ma) \
    ~E()                                 = d;      \
    constexpr E()                        = dc;     \
    constexpr E(E const &)               = cc;     \
    constexpr E(E &)                     = cc_;    \
    constexpr E(E &&)                    = mc;     \
    constexpr E & operator = (E const &) = ca;     \
    constexpr E & operator = (E &)       = ca_;    \
    constexpr E & operator = (E &&)      = ma;

template< bool >
struct enable_default_construct;

template<>
struct enable_default_construct< true >
{
    ENABLE(enable_default_construct,
           default,
           default,
           default,
           default,
           default,
           default,
           default,
           default
           )
};

template<>
struct enable_default_construct< false >
{
    ENABLE(enable_default_construct,
           default,
           delete,
           default,
           default,
           default,
           default,
           default,
           default
           )
};

template< bool >
struct enable_destruct;

template<>
struct enable_destruct< true >
{
    ENABLE(enable_destruct,
           default,
           default,
           default,
           default,
           default,
           default,
           default,
           default
           )
};

template<>
struct enable_destruct< false >
{
    ENABLE(enable_destruct,
           delete,
           default,
           default,
           default,
           default,
           default,
           default,
           default
           )
};

template< bool _copy, bool _move > // 2 * 2 == 2 + 2
struct enable_construct;

template<>
struct enable_construct< true, true >
{
    ENABLE(enable_construct,
           default,
           default,
           default,
           default,
           default,
           default,
           default,
           default
           )
};

template<>
struct enable_construct< false, false >
{
    ENABLE(enable_construct,
           default,
           default,
           delete,
           delete,
           delete,
           default,
           default,
           default
           )
};

template<>
struct enable_construct< true, false >
{
    ENABLE(enable_construct,
           default,
           default,
           default,
           default,
           delete,
           default,
           default,
           default
           )
};

template<>
struct enable_construct< false, true >
{
    ENABLE(enable_construct,
           default,
           default,
           delete,
           delete,
           default,
           default,
           default,
           default
           )
};

template< bool _copy, bool _move >
struct enable_assign;

template<>
struct enable_assign< true, true >
{
    ENABLE(enable_assign,
           default,
           default,
           default,
           default,
           default,
           default,
           default,
           default
           )
};

template<>
struct enable_assign< false, false >
{
    ENABLE(enable_assign,
           default,
           default,
           default,
           default,
           default,
           delete,
           delete,
           delete
           )
};

template<>
struct enable_assign< true, false >
{
    ENABLE(enable_assign,
           default,
           default,
           default,
           default,
           default,
           default,
           default,
           delete
           )
};

template<>
struct enable_assign< false, true >
{
    ENABLE(enable_assign,
           default,
           default,
           default,
           default,
           default,
           delete,
           delete,
           default
           )
};

template< bool d = true,
          bool dc = true,
          bool cc = true,
          bool mc = true,
          bool ca = cc,
          bool ma = mc >
struct enable_special_functions
        : enable_destruct< d >
        , enable_default_construct< dc >
        , enable_construct< cc, mc >
        , enable_assign< ca, ma >
{

};

template< typename from, typename to >
struct is_explicitly_convertible
        : std::integral_constant< bool, (std::is_constructible< to, from >{} && !std::is_convertible< from, to >{}) >
{

};

template< typename type, typename variant >
constexpr
bool
check_index(variant const & _variant) noexcept
{
    return (typename variant::template index< type >{} == _variant.which());
}

template< typename type, typename variant >
constexpr
bool
check_active(variant const & _variant) noexcept
{
    return _variant.template active< type >();
}

enum class state
{
    zero,
    default_constructed,
    value_copy_constructed,
    value_move_constructed,
    copy_constructed,
    move_constructed,
    move_assigned,
    copy_assigned,
    value_copy_assigned,
    value_move_assigned,
    moved_from,
    value_constructed
};

struct oracle_value // akrzemi1 optional test
{

    state s;
    int i;
    constexpr oracle_value(int _i = 0) : s(state::value_constructed), i(_i) { ; }

};

static_assert(std::is_literal_type< oracle_value >{});

struct oracle
{

    state s;
    oracle_value v;

    constexpr oracle() : s(state::default_constructed) { ; }
    constexpr oracle(oracle_value const & _v) : s(state::value_copy_constructed), v(_v) { ; }
    constexpr oracle(oracle_value && _v) : s(state::value_move_constructed), v(std::move(_v)) { _v.s = state::moved_from; }
    constexpr oracle(oracle const & o) : s(state::copy_constructed), v(o.v) {}
    constexpr oracle(oracle && o) : s(state::move_constructed), v(std::move(o.v)) { o.s = state::moved_from; }

    constexpr oracle & operator = (oracle_value const & _v) { s = state::value_copy_assigned; v = _v; return *this; }
    constexpr oracle & operator = (oracle_value && _v) { s = state::value_move_assigned; v = std::move(_v); v.s = state::moved_from; return *this; }
    constexpr oracle & operator = (oracle const & o) { s = state::copy_assigned; v = o.v; return *this; }
    constexpr oracle & operator = (oracle && o) { s = state::move_assigned; v = std::move(o.v); o.s = state::moved_from; return *this; }

    constexpr bool operator == (oracle const & o) const { return (o.v.i == v.i); }
    constexpr bool operator != (oracle const & o) const { return !operator == (o); }

};

static_assert(std::is_literal_type< oracle >{});

template< template< typename ... > class wrapper = identity >
class case_constexpr
{

    template< typename ...types >
    using V = variant< typename wrapper< types >::type... >;

    using VO = oracle_value;
    using O = oracle;

    static
    constexpr
    bool
    triviality() noexcept
    {
        struct A {};
        struct B {};
        {
            struct S {};
            ASSERT (__is_trivial(S));
            ASSERT (__has_trivial_destructor(S));
            ASSERT (__has_trivial_constructor(S));
            ASSERT (__has_trivial_copy(S));
            ASSERT (__has_trivial_assign(S));
            using VASB = V< A, S, B >;
            ASSERT (std::is_trivial< VASB >{});
            ASSERT (__has_trivial_destructor(VASB));
            ASSERT (__has_trivial_constructor(VASB));
            ASSERT (__has_trivial_copy(VASB));
            ASSERT (__has_trivial_assign(VASB));
        }
        {
            struct S { ~S() { ; } };
            ASSERT (std::is_destructible< S >{});
            ASSERT (!__has_trivial_destructor(S));
            using VASB = V< A, S, B >;
            ASSERT (std::is_destructible< VASB >{});
            ASSERT (!__has_trivial_destructor(VASB));
        }
        {
            struct S { S() { ; } };
            ASSERT (std::is_default_constructible< S >{});
            ASSERT (!__has_trivial_constructor(S));
            using VASB = V< A, S, B >;
            ASSERT (std::is_default_constructible< VASB >{});
            ASSERT (!__has_trivial_constructor(VASB));
        }
        {
            struct S { S(S const &) { ; } };
            ASSERT (std::is_copy_constructible< S >{});
            ASSERT (!__has_trivial_copy(S));
            using VASB = V< A, S, B >;
            ASSERT (!std::is_copy_constructible< VASB >{}); // non-trivial copyable alternative type prohibit copy
            ASSERT (!__has_trivial_copy(VASB));
        }
        {
            struct S { S & operator = (S const &) { return *this; } };
            ASSERT (std::is_copy_assignable< S >{});
            ASSERT (!__has_trivial_assign(S));
            using VASB = V< A, S, B >;
            ASSERT (!std::is_copy_assignable< VASB >{}); // non-trivial copy-assignable alternative type prohibit copy
            ASSERT (!__has_trivial_assign(VASB));
        }
        {
            struct A {};
            struct B { ~B() = delete; };
            using VAB = V< A, B >;
            using VBA = V< B, A >;
            ASSERT (!std::is_destructible< VAB >{});
            ASSERT (!std::is_destructible< VBA >{});
        }
        return true;
    }

    static
    constexpr
    bool
    default_constructor() noexcept
    {
        {
            struct II { II() { ; }     ~II() { ; } };
            struct OI { OI() = delete; ~OI() { ; } };
            struct IO { IO() { ; }     ~IO() = delete; };
            struct OO { OO() = delete; ~OO() = delete; };
            constexpr auto c_ii = __is_constructible(II);
            constexpr auto c_oi = __is_constructible(OI);
            constexpr auto c_io = __is_constructible(IO);
            constexpr auto c_oo = __is_constructible(OO);
            {
                using VII = V< II >;
                ASSERT (__is_constructible(VII) == c_ii);
            }
            {
                using VOI = V< OI >;
                ASSERT (__is_constructible(VOI) == c_oi);
            }
            {
                using VIO = V< IO >;
                ASSERT (__is_constructible(VIO) == c_io);
            }
            {
                using VOO = V< OO >;
                ASSERT (__is_constructible(VOO) == c_oo);
            }
        }
        {
            struct A { int i; };
            using VA = V< A >;
            constexpr VA v{};
            ASSERT (check_index< A >(v));
            ASSERT (check_active< A >(v));
            ASSERT (static_cast< A >(v).i == int{});
        }
        {
            struct A {};
            struct B { B() = delete; };
            using VAB = V< A, B >;
            using VBA = V< B, A >;
            ASSERT (!__is_constructible(VAB));
            ASSERT (!__is_constructible(VBA));
        }
        {
            struct A {};
            struct B {};
            using VAOB = variant< O, A, B >;
            VAOB v;
            CHECK (check_index< O >(v));
            CHECK (check_active< O >(v));
            CHECK (static_cast< O & >(v).s == state::default_constructed);
            VAOB const c{};
            CHECK (check_index< O >(c));
            CHECK (check_active< O >(c));
            CHECK (static_cast< O const & >(c).s == state::default_constructed);
        }
        {
            struct A { A() = delete; };
            struct B {};
            using VAOB = variant< A, O, B >;
            VAOB v;
            CHECK (check_index< O >(v));
            CHECK (check_active< O >(v));
            CHECK (static_cast< O & >(v).s == state::default_constructed);
            VAOB const c{};
            CHECK (check_index< O >(c));
            CHECK (check_active< O >(c));
            CHECK (static_cast< O const & >(c).s == state::default_constructed);
        }
        return true;
    }

    static
    constexpr
    bool
    conversion_constructor_and_conversion_operator() noexcept
    {
        struct A { int i = 1; };
        struct B { int i = 2; };
        struct C { int i = 3; };
        {
            using VAB = V< A, B >;
            CONSTEXPR VAB v{};
            ASSERT (static_cast< A const & >(v).i == 1);
            ASSERT (static_cast< A >(v).i == 1);
            CONSTEXPR VAB b{B{}};
            ASSERT (static_cast< B const & >(b).i == 2);
            ASSERT (static_cast< B >(b).i == 2);
            CONSTEXPR VAB a{A{3}};
            ASSERT (static_cast< A const & >(a).i == 3);
            ASSERT (static_cast< A >(a).i == 3);
        }
        {
            using VAB = V< A, B >;
            ASSERT (is_explicitly_convertible< VAB, A >{});
            ASSERT (is_explicitly_convertible< VAB, B >{});
            ASSERT (!is_explicitly_convertible< VAB, C >{});
            //A a = VAB{A{10}}; // implicit conversion is not allowed
            constexpr A a(VAB{A{10}}); // direct initialization is allowed
            ASSERT (a.i == 10);
            constexpr B b(VAB{B{20}}); // direct initialization is allowed
            ASSERT (b.i == 20);
        }
        {
            using VAOB = V< A, O, B >;
            O o;
            CHECK (o.s == state::default_constructed);
            VAOB v = o;
            CHECK (check_active< O >(v));
            CHECK (check_index< O >(v));
            CHECK (o.s == state::default_constructed);
            CHECK (static_cast< O & >(v).s == state::copy_constructed);
            CHECK (o.s == state::default_constructed);
            VAOB m = std::move(o);
            CHECK (o.s == state::moved_from);
            CHECK (check_active< O >(m));
            CHECK (check_index< O >(m));
            CHECK (static_cast< O & >(m).s == state::move_constructed);
        }
        {
            using VAOB = V< A, O, B >;
            VAOB v{O{}};
            CHECK (check_active< O >(v));
            CHECK (check_index< O >(v));
            CHECK (static_cast< O & >(v).s == state::move_constructed);
            O o = static_cast< O & >(v);
            CHECK (static_cast< O & >(v).s == state::move_constructed);
            CHECK (o.s == state::copy_constructed);
        }
        {
            using VAOB = V< A, O, B >;
            VAOB v{O{}};
            CHECK (check_active< O >(v));
            CHECK (check_index< O >(v));
            CHECK (static_cast< O & >(v).s == state::move_constructed);
            O o = static_cast< O && >(static_cast< O & >(v));
            CHECK (static_cast< O & >(v).s == state::moved_from);
            CHECK (o.s == state::move_constructed);
        }
        return true;
    }

    static
    constexpr
    bool
    conversion_assignment() noexcept
    { // http://stackoverflow.com/questions/33936295/
        {
            struct A {};
            struct B {};
            using VAB = V< A, B >;
            ASSERT (std::is_trivially_copy_assignable< VAB >{});
            VAB v;
            CHECK (check_active< A >(v));
            CHECK (check_index< A >(v));
            v = B{};
            CHECK (check_active< B >(v));
            CHECK (check_index< B >(v));
        }
        {
            using VX = variant< int, float, double >;
            ASSERT (std::is_trivially_copy_assignable< VX >{});
            VX x = 1;
            CHECK (check_active< int >(x));
            CHECK (check_index< int >(x));
            x = 1.0;
            CHECK (check_active< double >(x));
            CHECK (check_index< double >(x));
            x = 1.0f;
            CHECK (check_active< float >(x));
            CHECK (check_index< float >(x));
        }
        return true;
    }

    static
    constexpr
    bool
    copy_constructor_and_move_constructor() noexcept
    {
        struct A {};
        struct B {};
        struct C {};
        {
            struct NC {};
        }
        return true;
    }

    static
    constexpr
    bool
    copy_assignment_and_move_assignment() noexcept
    {
        struct A { int i = 1; };
        struct B { int j = 2; };
        struct C { int k = 3; };
        ASSERT (__has_trivial_copy(C));
        ASSERT (__has_trivial_assign(C));
        using VCBA = V< C, B, A >;
        ASSERT (std::is_trivially_copy_assignable< VCBA >{});
        {
            CONSTEXPR VCBA c{};
            ASSERT (check_active< C >(c));
            ASSERT (check_index< C >(c));
            CONSTEXPR VCBA b = B{};
            ASSERT (check_active< B >(b));
            ASSERT (check_index< B >(b));
            CONSTEXPR VCBA a = A{};
            ASSERT (check_active< A >(a));
            ASSERT (check_index< A >(a));
        }
        {
            VCBA x = A{};
            VCBA y = B{};
            VCBA z;
            VCBA w = x;
            CHECK (check_active< A >(w));
            CHECK (check_index< A >(w));
            x = y;
            CHECK (check_active< B >(x));
            CHECK (check_index< B >(x));
            CHECK (check_active< B >(y));
            CHECK (check_index< B >(y));
            y = z;
            CHECK (check_active< C >(y));
            CHECK (check_index< C >(y));
            CHECK (check_active< C >(z));
            CHECK (check_index< C >(z));
            z = w;
            CHECK (check_active< B >(x));
            CHECK (check_index< B >(x));
            CHECK (check_active< C >(y));
            CHECK (check_index< C >(y));
            CHECK (check_active< A >(z));
            CHECK (check_index< A >(z));
        }
        return true;
    }

    static
    constexpr
    bool
    emplace_constructor() noexcept
    {
        {
            struct A {};
            struct B {};
            struct S { constexpr S(A) { ; } };
            using VSAB = V< S, B, A >;
            CONSTEXPR VSAB b{};
            ASSERT (check_active< B >(b));
            ASSERT (check_index< B >(b));
            CONSTEXPR VSAB a{A{}};
            ASSERT (!check_active< S >(a));
            ASSERT (check_active< A >(a));
            ASSERT (check_index< A >(a));
        }
        {
            struct A {};
            struct B {};
            struct C { A a; };
            struct D { B b; };
            variant< aggregate_wrapper< C >, aggregate_wrapper< D > > c;
            CHECK (check_active< C >(c));
            CHECK (check_index< C >(c));
            variant< aggregate_wrapper< C >, aggregate_wrapper< D > > d{B{}};
            CHECK (check_active< D >(d));
            CHECK (check_index< D >(d));
        }
        return true;
    }

public :

    static
    constexpr
    bool
    run() noexcept
    {
        ASSERT (triviality());
        ASSERT (default_constructor());
        ASSERT (copy_constructor_and_move_constructor());
        ASSERT (copy_assignment_and_move_assignment());
        ASSERT (conversion_constructor_and_conversion_operator());
        ASSERT (conversion_assignment());
        ASSERT (emplace_constructor());
        return true;
    }

};

template< template< typename ... > class wrapper = identity >
class case_const
{

    template< typename ...types >
    using V = variant< typename wrapper< types >::type... >;

    static
    bool
    triviality() noexcept
    {
        return true;
    }

    static
    bool
    default_constructor() noexcept
    {
        {
            struct A { A() = delete; int i; };
            struct B { B() { ; } double d; };
            using VAB = V< A, B >;
            const VAB v{};
            assert ((check_active< B >(v)));
            assert ((check_index< B >(v)));
            assert (!(check_active< A >(v)));
            assert (!(check_index< A >(v)));
            assert (EQUIV (static_cast< B >(v).d, double{}));
        }
        {
            struct A { A() = delete; int i; };
            struct B { B() { ; } double d; };
            using VAB = V< A, B >;
            const VAB v{};
            assert ((check_active< B >(v)));
            assert ((check_index< B >(v)));
            assert (!(check_active< A >(v)));
            assert (!(check_index< A >(v)));
            assert (EQUIV (static_cast< B >(v).d, double{}));
        }
        return true;
    }

    static
    bool
    conversion_constructor_and_conversion_operator() noexcept
    {
        {
            struct A { int i = 1; };
            struct B { int i = 2; };
            using VAB = V< A, B >;
            VAB v;
            assert ((static_cast< A const & >(v).i == 1));
            assert ((static_cast< A & >(v).i == 1));
            static_cast< A & >(v) = A{2};
            assert ((static_cast< A & >(v).i == 2));
            VAB b{B{}};
            assert ((static_cast< B const & >(b).i == 2));
            assert ((static_cast< B & >(b).i == 2));
            static_cast< B & >(b) = B{1};
            assert ((static_cast< B & >(b).i == 1));
            VAB a{A{3}};
            assert ((static_cast< A const & >(a).i == 3));
            assert ((static_cast< A & >(a).i == 3));
            static_cast< A & >(a) = A{1};
            assert ((static_cast< A & >(a).i == 1));
        }
        return true;
    }

    static
    bool
    copy_assignment_and_move_assignment() noexcept
    {
        return true;
    }

public :

    static
    bool
    run() noexcept
    {
        assert ((triviality()));
        assert ((default_constructor()));
        assert ((conversion_constructor_and_conversion_operator()));
        assert ((copy_assignment_and_move_assignment()));
        return true;
    }

};

template< typename ...types >
struct aggregate_wrapper
        : identity< ::versatile::aggregate_wrapper< types >... >
{

};

} // namespace test
} // namespace versatile

#include <cstdlib>

int
main()
{
    ASSERT (versatile::test::case_constexpr<>::run());
    ASSERT (versatile::test::case_constexpr< ::versatile::test::aggregate_wrapper >::run());
    assert ((versatile::test::case_const<>::run()));
    assert ((versatile::test::case_const< ::versatile::test::aggregate_wrapper >::run()));
    return EXIT_SUCCESS;
}

#undef CHECK
#undef CONSTEXPR
#undef ASSERT

#else
#include <versatile.hpp>

#include <boost/variant.hpp>

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

#include <cstdlib>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

#ifndef COLS
#define COLS 5
#endif

#ifndef ROWS
#define ROWS COLS
#endif

namespace test
{

namespace test_traits
{

using namespace versatile;

struct A {};
struct B {};
static_assert(std::is_same< copy_cv_reference_t<          A         , B >,          B          >{});
static_assert(std::is_same< copy_cv_reference_t<          A const   , B >,          B const    >{});
static_assert(std::is_same< copy_cv_reference_t< volatile A         , B >, volatile B          >{});
static_assert(std::is_same< copy_cv_reference_t< volatile A const   , B >, volatile B const    >{});
static_assert(std::is_same< copy_cv_reference_t<          A        &, B >,          B        & >{});
static_assert(std::is_same< copy_cv_reference_t<          A const  &, B >,          B const  & >{});
static_assert(std::is_same< copy_cv_reference_t< volatile A        &, B >, volatile B        & >{});
static_assert(std::is_same< copy_cv_reference_t< volatile A const  &, B >, volatile B const  & >{});
static_assert(std::is_same< copy_cv_reference_t<          A       &&, B >,          B       && >{});
static_assert(std::is_same< copy_cv_reference_t<          A const &&, B >,          B const && >{});
static_assert(std::is_same< copy_cv_reference_t< volatile A       &&, B >, volatile B       && >{});
static_assert(std::is_same< copy_cv_reference_t< volatile A const &&, B >, volatile B const && >{});

}

struct introspector
{

    template< typename ...types >
    std::string
    operator () (types...) const
    {
        return __PRETTY_FUNCTION__;
    }

};

struct visitor0
{

    using R = std::array< bool, 3 >;

    template< typename type >
    constexpr
    R
    operator () (type &&) const
    {
        return {{true, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value}};
    }

    template< typename type >
    constexpr
    R
    operator () (type &&)
    {
        return {{false, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value}};
    }

};

struct visitor1
{

    using R = std::array< bool, 4 >;

    template< typename type >
    constexpr
    R
    operator () (type &&) const &
    {
        return {{true, true, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value}};
    }

    template< typename type >
    constexpr
    R
    operator () (type &&) &
    {
        return {{false, true, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value}};
    }

    template< typename type >
    constexpr
    R
    operator () (type &&) &&
    {
        return {{false, false, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value}};
    }

};

struct visitor2
{

    template< typename ...types >
    constexpr
    auto
    operator () (types &&... _values) const
    {
        return std::tuple_cat(std::make_tuple(true), std::make_tuple(std::tie(typeid(std::forward< types >(_values))), std::is_const< std::remove_reference_t< types > >::value, std::is_lvalue_reference< types >::value)...);
    }

    template< typename ...types >
    constexpr
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
    constexpr
    R
    operator () (type &&) const &
    {
        return std::make_tuple(true, true, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value);
    }

    template< typename type >
    constexpr
    R
    operator () (type &&) &
    {
        return std::make_tuple(false, true, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value);
    }

    template< typename type >
    constexpr
    R
    operator () (type &&) const &&
    {
        return std::make_tuple(true, false, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value);
    }

    template< typename type >
    constexpr
    R
    operator () (type &&) &&
    {
        return std::make_tuple(false, false, std::is_const< std::remove_reference_t< type > >::value, std::is_lvalue_reference< type >::value);
    }

};

// value_or
template< typename lhs, typename rhs >
std::enable_if_t< (versatile::is_visitable< std::remove_reference_t< lhs > >{} && !versatile::is_visitable< std::remove_reference_t< rhs > >{}), std::remove_reference_t< rhs > >
operator || (lhs && _lhs, rhs && _rhs) noexcept
{
    using result_type = std::remove_reference_t< rhs >;
    if (_lhs.template active< result_type >()) {
        return static_cast< result_type >(std::forward< lhs >(_lhs));
    } else {
        return std::forward< rhs >(_rhs);
    }
}

template< typename lhs, typename rhs >
std::enable_if_t< (!versatile::is_visitable< std::remove_reference_t< lhs > >{} && versatile::is_visitable< std::remove_reference_t< rhs > >{}), lhs >
operator || (lhs && _lhs, rhs && _rhs) noexcept
{
    return (std::forward< rhs >(_rhs) || std::forward< lhs >(_lhs));
}

template< typename lhs, typename rhs >
std::enable_if_t< (versatile::is_visitable< std::remove_reference_t< lhs > >{} && versatile::is_visitable< std::remove_reference_t< rhs > >{}) >
operator || (lhs && _lhs, rhs && _rhs) = delete;

template< typename type >
using same_t = type;

template< typename variadic, typename type >
struct variadic_index;

template< template< typename ...types > class variadic, typename ...types,
          typename type >
struct variadic_index< variadic< types... >, type >
        : versatile::index_at< type, types..., void >
{

};

template< bool ...values >
struct index
{

};

template< bool ...rest >
struct index< true, rest... >
        : std::integral_constant< std::size_t, sizeof...(rest) >
{

};

template< bool ...rest >
struct index< false, rest... >
        : index< rest... >
{

};

template< std::size_t _id >
struct id
{

};

template< bool, typename ...types >
struct enable_default_construct;

template< typename ...types >
struct enable_default_construct< true, types... >
{

    enable_default_construct() = default;
    enable_default_construct(enable_default_construct const &) = default;
    enable_default_construct(enable_default_construct &) = default;
    enable_default_construct(enable_default_construct &&) = default;
    enable_default_construct & operator = (enable_default_construct const &) = default;
    enable_default_construct & operator = (enable_default_construct &) = default;
    enable_default_construct & operator = (enable_default_construct &&) = default;

    std::size_t which_ = 1 + index< (std::is_default_constructible< types >{})... >{};

};

template< typename ...types >
struct enable_default_construct< false, types... >
{

    enable_default_construct() = delete;
    enable_default_construct(enable_default_construct const &) = default;
    enable_default_construct(enable_default_construct &) = default;
    enable_default_construct(enable_default_construct &&) = default;
    enable_default_construct & operator = (enable_default_construct const &) = default;
    enable_default_construct & operator = (enable_default_construct &) = default;
    enable_default_construct & operator = (enable_default_construct &&) = default;

    std::size_t which_;

};

template< bool >
struct enable_destruct;

template<>
struct enable_destruct< true >
{

    ~enable_destruct() = default;
    enable_destruct() = default;
    enable_destruct(enable_destruct const &) = default;
    enable_destruct(enable_destruct &) = default;
    enable_destruct(enable_destruct &&) = default;
    enable_destruct & operator = (enable_destruct const &) = default;
    enable_destruct & operator = (enable_destruct &) = default;
    enable_destruct & operator = (enable_destruct &&) = default;

};

template<>
struct enable_destruct< false >
{

    ~enable_destruct() = delete;
    enable_destruct() = default;
    enable_destruct(enable_destruct const &) = default;
    enable_destruct(enable_destruct &) = default;
    enable_destruct(enable_destruct &&) = default;
    enable_destruct & operator = (enable_destruct const &) = default;
    enable_destruct & operator = (enable_destruct &) = default;
    enable_destruct & operator = (enable_destruct &&) = default;

};

template< bool _copy, bool _move > // 2 * 2 == 2 + 2
struct enable_construct;

template<>
struct enable_construct< true, true >
{

    ~enable_construct() = default;
    enable_construct() = default;
    enable_construct(enable_construct const &) = default;
    enable_construct(enable_construct &) = default;
    enable_construct(enable_construct &&) = default;
    enable_construct & operator = (enable_construct const &) = default;
    enable_construct & operator = (enable_construct &) = default;
    enable_construct & operator = (enable_construct &&) = default;

};

template<>
struct enable_construct< false, false >
{

    ~enable_construct() = default;
    enable_construct() = default;
    enable_construct(enable_construct const &) = delete;
    enable_construct(enable_construct &) = delete;
    enable_construct(enable_construct &&) = delete;
    enable_construct & operator = (enable_construct const &) = default;
    enable_construct & operator = (enable_construct &) = default;
    enable_construct & operator = (enable_construct &&) = default;

};

template<>
struct enable_construct< true, false >
{

    ~enable_construct() = default;
    enable_construct() = default;
    enable_construct(enable_construct const &) = default;
    enable_construct(enable_construct &) = default;
    enable_construct(enable_construct &&) = delete;
    enable_construct & operator = (enable_construct const &) = default;
    enable_construct & operator = (enable_construct &) = default;
    enable_construct & operator = (enable_construct &&) = default;

};

template<>
struct enable_construct< false, true >
{

    ~enable_construct() = default;
    enable_construct() = default;
    enable_construct(enable_construct const &) = delete;
    enable_construct(enable_construct &) = delete;
    enable_construct(enable_construct &&) = default;
    enable_construct & operator = (enable_construct const &) = default;
    enable_construct & operator = (enable_construct &) = default;
    enable_construct & operator = (enable_construct &&) = default;

};

template< bool _copy, bool _move >
struct enable_assign;

template<>
struct enable_assign< true, true >
{

    ~enable_assign() = default;
    enable_assign() = default;
    enable_assign(enable_assign const &) = default;
    enable_assign(enable_assign &) = default;
    enable_assign(enable_assign &&) = default;
    enable_assign & operator = (enable_assign const &) = default;
    enable_assign & operator = (enable_assign &) = default;
    enable_assign & operator = (enable_assign &&) = default;

};

template<>
struct enable_assign< false, false >
{

    ~enable_assign() = default;
    enable_assign() = default;
    enable_assign(enable_assign const &) = default;
    enable_assign(enable_assign &) = default;
    enable_assign(enable_assign &&) = default;
    enable_assign & operator = (enable_assign const &) = delete;
    enable_assign & operator = (enable_assign &) = delete;
    enable_assign & operator = (enable_assign &&) = delete;

};

template<>
struct enable_assign< true, false >
{

    ~enable_assign() = default;
    enable_assign() = default;
    enable_assign(enable_assign const &) = default;
    enable_assign(enable_assign &) = default;
    enable_assign(enable_assign &&) = default;
    enable_assign & operator = (enable_assign const &) = default;
    enable_assign & operator = (enable_assign &) = default;
    enable_assign & operator = (enable_assign &&) = delete;

};

template<>
struct enable_assign< false, true >
{

    ~enable_assign() = default;
    enable_assign() = default;
    enable_assign(enable_assign const &) = default;
    enable_assign(enable_assign &) = default;
    enable_assign(enable_assign &&) = default;
    enable_assign & operator = (enable_assign const &) = delete;
    enable_assign & operator = (enable_assign &) = delete;
    enable_assign & operator = (enable_assign &&) = default;

};

template< typename ...types >
using enable_default_construct_t = enable_default_construct< (std::is_default_constructible< types >{} || ...), types... >;

template< typename ...types >
using enable_destruct_t = enable_destruct< (std::is_destructible< types >{} && ...) >;

template< typename ...types >
using enable_construct_t = enable_construct< (std::is_copy_constructible< types >{} && ...), (std::is_move_constructible< types >{} && ...) >;

template< typename ...types >
using enable_assign_t = enable_assign< (std::is_copy_assignable< types >{} && ...), (std::is_move_assignable< types >{} && ...) >;

template< typename ...types >
struct enable_special_functions
        : enable_default_construct_t< types... >
        , enable_destruct_t< types... >
        , enable_construct_t< types... >
        , enable_assign_t< types... >
{

    using base = enable_default_construct_t< types... >;
    using base::which_;

    template< std::size_t _id >
    constexpr
    enable_special_functions(id< _id >)
        : base{_id}
    { ; }

};


template< typename ...types >
struct cvariant
        : enable_special_functions< versatile::unwrap_type_t< types >... >
{

    static constexpr std::size_t width = sizeof...(types);

    using base = enable_special_functions< versatile::unwrap_type_t< types >... >;

    ~cvariant() = default;
    cvariant() = default;
    cvariant(cvariant const &) = default;
    cvariant(cvariant &) = default;
    cvariant(cvariant &&) = default;
    cvariant & operator = (cvariant const &) = default;
    cvariant & operator = (cvariant &) = default;
    cvariant & operator = (cvariant &&) = default;

    constexpr std::size_t which() const { return base::which_; }

    template< typename type >
    using index_at_t = versatile::index_at< versatile::unwrap_type_t< type >, versatile::unwrap_type_t< types >..., void >;

    template< typename type, std::size_t _which = index_at_t< type >{} >
    constexpr cvariant(type &&) : base{id< _which >{}} { ; }

    template< typename ...arguments, std::size_t _which = 1 + index< (std::is_constructible< versatile::unwrap_type_t< types >, arguments... >{})... >{} >
    constexpr cvariant(arguments &&...) : base{id< _which >{}} { ; }

    template< typename type, std::size_t = index_at_t< type >{} >
    constexpr operator type const & () const
    {
        return value< type >;
    }

    template< typename type, std::size_t = index_at_t< type >{} >
    constexpr operator type & ()
    {
        return value< type >;
    }

private :

    template< typename type >
    static type value;

};

template< typename ...types >
template< typename type >
type cvariant< types... >::value = {};

} // namespace test

namespace versatile
{

template< typename first, typename ...rest >
struct is_visitable< test::cvariant< first, rest... > >
        : std::true_type
{

};

}

namespace test
{

template< typename F, std::size_t ...indices >
struct enumerator;

template< typename F >
struct enumerator< F >
{

    constexpr
    enumerator(F && _f)
        : f(std::forward< F >(_f))
    { ; }

    template< std::size_t ...I >
    constexpr
    bool
    operator () () const
    {
        return f(std::index_sequence< I... >{});
    }

private :

    F f;

};

template< typename F, std::size_t first, std::size_t ...rest >
struct enumerator< F, first, rest... >
        : enumerator< F, rest... >
{

    using base = enumerator< F, rest... >;

    constexpr
    enumerator(F && _f)
        : base(std::forward< F >(_f))
    { ; }

    template< std::size_t ...I >
    constexpr
    bool
    operator () () const
    {
        return enumerator::template operator () < I... >(std::make_index_sequence< first >{}); // ltr
    }

    template< std::size_t ...I, std::size_t ...J >
    constexpr
    bool
    operator () (std::index_sequence< J... >) const
    {
        return (base::template operator () < I..., J >() && ...); // rtl, `< J, I... >` - ltr
    }

};

template< std::size_t ...indices, typename F >
constexpr
enumerator< F, indices... >
make_enumerator(F && _f)
{
    static_assert(0 < sizeof...(indices));
    static_assert(((0 < indices) && ...));
    return std::forward< F >(_f);
}

template< std::size_t I >
struct T
{

    constexpr
    operator std::size_t () const noexcept
    {
        return I;
    }

};

template< std::size_t M >
struct pair
{

    std::array< versatile::type_qualifier, (1 + M) > qual_ids;
    std::array< std::size_t, (1 + M) > type_ids;

    template< std::size_t ...I >
    constexpr
    bool
    ids_equal(pair const & _rhs, std::index_sequence< I... >) const
    {
        return ((qual_ids[I] == _rhs.qual_ids[I]) && ...) && ((type_ids[I] == _rhs.type_ids[I]) && ...);
    }

    constexpr
    bool
    operator == (pair const & _rhs) const
    {
        return ids_equal(_rhs, std::make_index_sequence< 1 + M >{});
    }

    constexpr
    std::size_t
    size() const
    {
        return (1 + M);
    }

};

template< std::size_t M >
struct multivisitor
{

    using result_type = pair< M >;

    constexpr
    std::size_t
    which() const
    {
        return 0;
    }

    static constexpr std::size_t width = M;

    template< typename ...types >
    constexpr
    result_type
    operator () (types &&... _values) & noexcept
    {
        //static_assert(M == sizeof...(types));
        //static_assert(!(is_visitable< types >{} || ...));
        return {{{versatile::type_qualifier_of< multivisitor & >, versatile::type_qualifier_of< types && >...}}, {{M, _values...}}};
    }

    template< typename ...types >
    constexpr
    result_type
    operator () (types &&... _values) const & noexcept
    {
        return {{{versatile::type_qualifier_of< multivisitor const & >, versatile::type_qualifier_of< types && >...}}, {{M, _values...}}};
    }

    template< typename ...types >
    constexpr
    result_type
    operator () (types &&... _values) && noexcept
    {
        return {{{versatile::type_qualifier_of< multivisitor && >, versatile::type_qualifier_of< types && >...}}, {{M, _values...}}};
    }

    template< typename ...types >
    constexpr
    result_type
    operator () (types &&... _values) const && noexcept
    {
        return {{{versatile::type_qualifier_of< multivisitor const && >, versatile::type_qualifier_of< types && >...}}, {{M, _values...}}};
    }

#if 0
    //volatile qualifier not properly supported in STL

    template< typename ...types >
    constexpr
    result_type
    operator () (types &&... _values) volatile & noexcept
    {
        return {{{versatile::type_qualifier_of< volatile multivisitor & >, versatile::type_qualifier_of< types && >...}}, {{M, _values...}}};
    }

    template< typename ...types >
    constexpr
    result_type
    operator () (types &&... _values) volatile const & noexcept
    {
        return {{{versatile::type_qualifier_of< volatile multivisitor const & >, versatile::type_qualifier_of< types && >...}}, {{M, _values...}}};
    }

    template< typename ...types >
    constexpr
    result_type
    operator () (types &&... _values) volatile && noexcept
    {
        return {{{versatile::type_qualifier_of< volatile multivisitor && >, versatile::type_qualifier_of< types && >...}}, {{M, _values...}}};
    }

    template< typename ...types >
    constexpr
    result_type
    operator () (types &&... _values) volatile const && noexcept
    {
        return {{{versatile::type_qualifier_of< volatile multivisitor const && >, versatile::type_qualifier_of< types && >...}}, {{M, _values...}}};
    }
#endif

};

static constexpr std::size_t qualifier_id_begin = static_cast< std::size_t >(versatile::type_qualifier_of< void * & >);
static constexpr std::size_t qualifier_id_end = static_cast< std::size_t >(versatile::type_qualifier_of< void * volatile & >);

template< typename ...types >
struct fusor
{

    std::tuple< types *... > const & stuff_;

    template< std::size_t ...Q >
    constexpr
    bool
    operator () (std::index_sequence< Q... >) const
    {
        return call< Q... >(std::index_sequence_for< types... >{});
    }

private :

    template< std::size_t ...Q, std::size_t ...K >
    constexpr
    bool
    call(std::index_sequence< K... >) const
    {
        auto const lhs = versatile::multivisit(static_cast< versatile::add_qualifier_t< static_cast< versatile::type_qualifier >(qualifier_id_begin + Q), types > >(*std::get< K >(stuff_))...);
        static_assert(sizeof...(types) == lhs.size());
        pair< (sizeof...(types) - 1) > const rhs = {{{static_cast< versatile::type_qualifier >(qualifier_id_begin + Q)...}}, {{(std::get< K >(stuff_)->width - 1 - std::get< K >(stuff_)->which())...}}};
        if (lhs == rhs) {
            return false;
        }
        return true;
    }

};

template< typename ...types >
constexpr
fusor< types... >
make_fusor(std::tuple< types *... > const & _stuff)
{
    static_assert(((versatile::type_qualifier_of< types > == versatile::type_qualifier::value) && ...));
    return {_stuff};
}

template< template< typename ...types > class variant,
          std::size_t M, std::size_t N = M > // M - multivisitor arity, N - number of alternative (bounded) types
class test_perferct_forwarding
{

    template< typename >
    struct variants;

    template< std::size_t ...J >
    struct variants< std::index_sequence< J... > >
    {

        using variant_type = variant< T< J >... >;
        static_assert(N == variant_type::width);

        variant_type variants_[N] = {T< J >{}...};

    };

    template< std::size_t ...I >
    constexpr
    bool
    operator () (std::index_sequence< I... >) const
    {
        static_assert(sizeof...(I) == M);
        std::size_t indices[M] = {};
        for (std::size_t & m : indices) {
            m = 0;
        }
        multivisitor< M > mv;
        variants< std::make_index_sequence< N > > variants_;
        auto permutation_ = std::make_tuple(&mv, &variants_.variants_[indices[I]]...);
        auto const fusor_ = make_fusor(permutation_);
        constexpr std::size_t ref_count = (qualifier_id_end - qualifier_id_begin); // test only reference types
        auto const enumerator_ = make_enumerator< ref_count, (I, ref_count)... >(fusor_);
        for (;;) {
            { // constexpr version of `permutation_ = std::make_tuple(&mv, &variants_.variants_[indices[I]]...);`
                std::get< 0 >(permutation_) = &mv;
                ((std::get< 1 + I >(permutation_) = &variants_.variants_[indices[I]]), ...);
            }
            if (!enumerator_()) {
                return false;
            }
            std::size_t m = 0;
            for (;;) {
                std::size_t & n = indices[m];
                ++n;
                if (n != N) {
                    break;
                }
                n = 0;
                if (++m == M) {
                    return true;
                }
            }
        }
        return true;
    }

public :

    constexpr
    bool
    operator () () const
    {
        return operator () (std::make_index_sequence< M >{});
    }

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
    return (std::array< std::size_t, sizeof...(N) >{(N % sizeof...(M))...} == multivisit(visitor{}, versatile::variant< T< M >... >{T< (N % sizeof...(M)) >{}}...));
}

#pragma clang diagnostic pop

template< std::size_t M, std::size_t N = M >
constexpr
bool
hard() noexcept
{
    return invoke(std::make_index_sequence< M >{}, std::make_index_sequence< N >{});
}

inline int f() { return 1; }
inline int g() { return 2; }

template< typename ...types >
struct boost_variant_i
        : boost::variant< types... >
{

    using base = boost::variant< types... >;

    using base::base;
    using base::operator =;

    std::size_t
    which() const
    {
        return (sizeof...(types) - static_cast< std::size_t >(base::which()));
    }

    template< typename type >
    static
    constexpr
    std::size_t
    index() noexcept
    {
        return versatile::index_at< versatile::unwrap_type_t< type >, versatile::unwrap_type_t< types >..., void >();
    }

    template< typename type >
    bool
    active() const noexcept
    {
        return (index< type >() == which());
    }

    template< typename type >
    explicit
    operator type & () &
    {
        if (!active< type >()) {
            throw std::bad_cast{};
        }
        return boost::get< type & >(static_cast< boost_variant_i::base & >(*this));
    }

    template< typename type >
    explicit
    operator type const & () const &
    {
        if (!active< type >()) {
            throw std::bad_cast{};
        }
        return boost::get< type const & >(static_cast< boost_variant_i::base const & >(*this));
    }

    template< typename type >
    explicit
    operator type && () &&
    {
        if (!active< type >()) {
            throw std::bad_cast{};
        }
        return boost::get< type && >(static_cast< boost_variant_i::base && >(*this));
    }

    template< typename type >
    explicit
    operator type const && () const &&
    {
        if (!active< type >()) {
            throw std::bad_cast{};
        }
        return boost::get< type const && >(static_cast< boost_variant_i::base const & >(*this));
    }

};

template< typename ...types >
struct boost_variant_c
{

    using variant = boost::variant< types... >;

    boost_variant_c(boost_variant_c &) = default;
    boost_variant_c(boost_variant_c const &) = default;
    boost_variant_c(boost_variant_c &&) = default;

    boost_variant_c(boost_variant_c const && _rhs)
        : v(std::move(_rhs.v))
    { ; }

    template< typename ...arguments >
    boost_variant_c(arguments &&... _arguments)
        : v(std::forward< arguments >(_arguments)...)
    { ; }

    boost_variant_c &
    operator = (boost_variant_c const &) = default;
    boost_variant_c &
    operator = (boost_variant_c &) = default;
    boost_variant_c &
    operator = (boost_variant_c &&) = default;

    boost_variant_c &
    operator = (boost_variant_c const && _rhs)
    {
        v = std::move(_rhs.v);
        return *this;
    }

    template< typename argument >
    boost_variant_c &
    operator = (argument && _argument)
    {
        v = std::forward< argument >(_argument);
        return *this;
    }

    std::size_t
    which() const
    {
        return (sizeof...(types) - static_cast< std::size_t >(v.which()));
    }

    template< typename type >
    static
    constexpr
    std::size_t
    index() noexcept
    {
        return versatile::index_at< versatile::unwrap_type_t< type >, versatile::unwrap_type_t< types >..., void >();
    }

    template< typename type >
    bool
    active() const noexcept
    {
        return (index< type >() == which());
    }

    template< typename type >
    explicit
    operator type & () &
    {
        if (!active< type >()) {
            throw std::bad_cast{};
        }
        return boost::get< type & >(v);
    }

    template< typename type >
    explicit
    operator type const & () const &
    {
        if (!active< type >()) {
            throw std::bad_cast{};
        }
        return boost::get< type const & >(v);
    }

    template< typename type >
    explicit
    operator type && () &&
    {
        if (!active< type >()) {
            throw std::bad_cast{};
        }
        return boost::get< type && >(v);
    }

    template< typename type >
    explicit
    operator type const && () const &&
    {
        if (!active< type >()) {
            throw std::bad_cast{};
        }
        return boost::get< type const && >(v);
    }

private :

    boost::variant< types... > v;

};

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

namespace versatile
{

template< typename type >
struct unwrap_type< boost::recursive_wrapper< type > >
        : unwrap_type< type >
{

};

template< typename first, typename ...rest >
struct is_visitable< test::boost_variant_i< first, rest... > >
        : std::true_type
{

};

template< typename first, typename ...rest >
struct is_visitable< test::boost_variant_c< first, rest... > >
        : std::true_type
{

};

template< typename type >
struct unwrap_type< std::reference_wrapper< type > >
        : unwrap_type< type >
{

    static_assert(!std::is_const< type >{});

};

} // namespace versatile

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
            assert(static_cast< int >(w) == int{});
            V v{2};
            assert(static_cast< int >(v) == 2);
        }
        {
            using V = versatile< std::true_type, std::false_type >;
            V t;
            assert(static_cast< std::true_type >(t) == std::true_type{});
            V f{std::false_type{}};
            assert(static_cast< std::false_type >(f) == std::false_type{});
        }
        {
            using V = variant< std::true_type, std::false_type >;
            V t;
            assert(static_cast< std::true_type >(t) == std::true_type{});
            V f{std::false_type{}};
            assert(static_cast< std::false_type >(f) == std::false_type{});
        }
#if 0
        { // trivial
            using V = versatile< int, double >;
            //static_assert(std::is_trivially_copy_constructible< V >{}); // ???
            //static_assert(std::is_trivially_move_constructible< V >{}); // ???
            static_assert(std::is_trivially_copy_assignable< V >{});
            static_assert(std::is_trivially_move_assignable< V >{});
            V v{1.0};
            assert(v.active< double >());
            V u{v};
            assert(u.active< double >());
            V w{1};
            v = w;
            assert(v.active< int >());
        }
#endif
#if 0
        { // non-standard layout (really UB http://talesofcpp.fusionfenix.com/post-21/)
            struct A { int i; };
            struct B : A { A a; B(int i, int j) : A{i}, a{j} { ; } };
            static_assert(!std::is_standard_layout< B >{});
            using V = versatile< A, B >;
            static_assert(!std::is_standard_layout< V >{});
            V v{B{1, 2}};
            assert(v.active< B >());
            assert(static_cast< B >(v).A::i == 1);
            assert(static_cast< B >(v).a.i == 2);
        }
        { // non-standard layout (really UB http://talesofcpp.fusionfenix.com/post-21/)
            struct A { int i; A(int j) : i(j) { ; } };
            struct B : A { using A::A; };
            struct C : A { using A::A; };
            struct D : B, C { D(int i, int j) : B{i}, C{j} { ; } };
            static_assert(!std::is_standard_layout< D >{});
            using V = versatile< A, B, C, D >;
            static_assert(!std::is_standard_layout< V >{});
            V v{D{1, 2}};
            assert(v.active< D >());
            assert(static_cast< D >(v).B::i == 1);
            assert(static_cast< D >(v).C::i == 2);
        }
#endif
        {
            using V = variant< int >;
            V v{2};
            assert(static_cast< int >(v) == 2);
        }
        { // value_or
            using V = variant< double, int >;
            {
                V D{1.0};
                int i = D || 2;
                assert(i == 2);
                V I{1};
                int j = I || 2;
                assert(j == 1);
            }
            {
                int i = 2 || V{1.0};
                assert(i == 2);
                int j = 2 || V{1} ;
                assert(j == 1);
            }
        }
        {
            using std::swap;
            using V = variant< int >;
            static_assert(V::width == 1, "V::width != 1");
            V v;
            static_assert(1 == variadic_index< V, int >{});
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
            swap(v, w);
            assert(static_cast< int >(w) == 223);
            assert(static_cast< int >(v) == 222);
            swap(w, v);
            assert(static_cast< int >(w) == 222);
            assert(static_cast< int >(v) == 223);
            V u = w;
            assert(static_cast< int >(u) == 222);
            assert(w.active< int >());
            static_assert(variadic_index< V, int >{} == 1);
            assert(u.active< int >());
        }
        {
            using V = variant< int, float, double, long double >;
            assert((V{}.which() == variadic_index< V, int >{}));
            assert((V{0}.which() == variadic_index< V, int >{}));
            assert((V{1.0f}.which() == variadic_index< V, float >{}));
            assert((V{2.0}.which() == variadic_index< V, double >{}));
            assert((V{3.0L}.which() == variadic_index< V, long double >{}));
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
            using std::swap;
            swap(d, j);
            assert(d.active< int >());
            assert(j.active< float >());
        }
        {
            struct A { A() = delete; };
            struct B {};
            using V =  variant< A, B >;
            V v;
            assert(v.active< B >());
            v = A{};
            assert(v.active< A >());
            V w{A{}};
            assert(w.active< A >());
        }
        { // incomplete
            struct A {};
            struct B;
            using V = variant< A, RW< B > >;
            V v;
            assert(v.active< A >());
            struct B {}; // if declared but not defined then there is compilation error in std::unique_ptr destructor
            v = B{};
            assert(v.active< B >());
            assert(v.active< RW< B > >());
        }
        { // recursive (composition)
            struct R;
            struct A {};
            using V = variant< A, RW< R > >;
            V v;
            assert(v.active< A >());
            struct R { V v; };
            v = R{};
            assert(v.active< R >());
            assert(v.active< RW< R > >());
            assert(static_cast< R & >(v).v.active< A >());
        }
        { // recursive (inheritance)
            struct R;
            struct A {};
            using V = variant< A, RW< R > >;
            V u;
            assert(u.active< A >());
            struct R : V { using V::V; using V::operator =; };
            u = R{};
            assert(u.active< R >());
            assert(u.active< RW< R > >());
            R v;
            assert(v.active< A >());
            v = R{};
            assert(v.active< A >());
            R w{R{}};
            assert(w.active< A >());
            R x{V{R{}}};
            assert(x.active< R >());
            assert(x.active< RW< R > >());
        }
        { // exact
            using V = variant< int, bool >;
            assert(V{} == V{});
            assert(V{} == int{});
            assert(int{} == V{});
            assert(V{1} == 1);
            assert(1 == V{1});
            assert(V{false} == false);
            assert(true == V{true});
        }
        { // relational
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
            assert(x == A{0});
            assert(y == A{1});
        }
        {
            struct A { A(int &, int) {} };
            struct B { B(int const &, int) {} };
            using V = variant< A, B >;
            V v{in_place, 1, 2};
            assert(v.active< B >());
            int i{1};
            V w{in_place, i, 2};
            assert(w.active< A >());
        }
        {
            variant< char const * > v;
            assert(static_cast< char const * >(v) == nullptr);
        }
        {
            struct A {};
            struct B { B(B &&) = default; B(B const &) = delete; B & operator = (B &&) = default; void operator = (B const &) = delete; };
            variant< B, A > v;
            assert(v.active< A >());
            v = B{};
            assert(v.active< B >());
        }
        {
            struct A { A(double) { ; } };
            using V = variant< A, int >;
            assert(V(in_place, int{0}).active< A >());
            assert(V{int{0}}.active< int >());
        }
        {
            struct B;
            struct A { A(B const &) { ; } };
            struct B {};
            using V = variant< A, B >;
            assert(V{}.active< B >());
            assert(V{B{}}.active< B >());
            assert((V{in_place, B{}}.active< A >()));
        }
        {
            static int counter = 0;
            struct A {};
            struct B { int i; B(int j) : i(j) { ++counter; } B() = delete; B(B const &) = delete; B(B &&) = delete; };
            struct C {};
            B b{1};
            assert(counter == 1);
            using V = variant< A, B, C >;
            V v;
            assert(v.active< A >());
            assert(counter == 1);
            v.emplace(2);
            assert(v.active< B >());
            assert(static_cast< B const & >(v).i == 2);
            assert(counter == 2);
            v.emplace(1);
            assert(v.active< B >());
            assert(static_cast< B const & >(v).i == 1);
            assert(counter == 3);
        }
        {
            struct A {};
            using V = variant< RW< A > >;
            visitor0 p_;
            visitor0 const cp_{};
            V v;
            V const cv;
            using B3 = std::array< bool, 3 >;
            assert((visit(p_, v)    == B3{{false, false, true }}));
            assert((visit(cp_, v)   == B3{{true,  false, true }}));
            assert((visit(p_, cv)   == B3{{false, true,  true }}));
            assert((visit(cp_, cv)  == B3{{true,  true,  true }}));
            assert((visit(p_, V())  == B3{{false, false, false}}));
            assert((visit(cp_, V()) == B3{{true,  false, false}}));
        }
        {
            struct A {};
            using V = variant< RW< A > >;
            visitor1 p_;
            visitor1 const cp_{};
            V v;
            V const cv{};
            using B4 = std::array< bool, 4 >;
            assert((visit(p_, v)           == B4{{false, true,  false, true }}));
            assert((visit(cp_, v)          == B4{{true,  true,  false, true }}));
            assert((visit(visitor1{}, v)   == B4{{false, false, false, true }}));
            assert((visit(p_, cv)          == B4{{false, true,  true,  true }}));
            assert((visit(cp_, cv)         == B4{{true,  true,  true,  true }}));
            assert((visit(visitor1{}, cv)  == B4{{false, false, true,  true }}));
            assert((visit(p_, V())         == B4{{false, true,  false, false}}));
            assert((visit(cp_, V())        == B4{{true,  true,  false, false}}));
            assert((visit(visitor1{}, V()) == B4{{false, false, false, false}}));
        }
        { // multivisitation with forwarding
            struct A {};
            struct B {};
            using V = variant< RW< A >, RW< B > >;
            visitor2 v_;
            visitor2 const c_{};
            assert(multivisit(v_, V{B{}}) == std::make_tuple(false, std::tie(typeid(B)), false, false));
            assert(multivisit(c_, V{B{}}) == std::make_tuple(true, std::tie(typeid(B)), false, false));
            assert(multivisit(visitor2{}, V{B{}}) == std::make_tuple(false, std::tie(typeid(B)), false, false));
            assert(multivisit(v_, V{}, V{B{}}) == std::make_tuple(false, std::tie(typeid(A)), false, false, std::tie(typeid(B)), false, false));
            assert(multivisit(c_, V{}, V{B{}}) == std::make_tuple(true, std::tie(typeid(A)), false, false, std::tie(typeid(B)), false, false));
            assert(multivisit(visitor2{}, V{}, V{B{}}) == std::make_tuple(false, std::tie(typeid(A)), false, false, std::tie(typeid(B)), false, false));
            // forwarding
            enum class op { eq, lt, gt, nge };
            assert(multivisit(v_, V{}, op::eq, V{B{}}) == std::make_tuple(false, std::tie(typeid(A)), false, false, std::tie(typeid(op)), false, false, std::tie(typeid(B)), false, false));
            assert(multivisit(c_, V{}, op::eq, V{B{}}) == std::make_tuple(true, std::tie(typeid(A)), false, false, std::tie(typeid(op)), false, false, std::tie(typeid(B)), false, false));
            assert(multivisit(visitor2{}, V{}, op::eq, V{B{}}) == std::make_tuple(false, std::tie(typeid(A)), false, false, std::tie(typeid(op)), false, false, std::tie(typeid(B)), false, false));
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

            assert(d0(v) ==   std::make_tuple(false, std::tie(typeid(A)), false, true ));
            assert(d0(c) ==   std::make_tuple(false, std::tie(typeid(B)), true,  true ));
            assert(d0(V{}) == std::make_tuple(false, std::tie(typeid(A)), false, false));

            assert(d1(v) ==   std::make_tuple(true, std::tie(typeid(A)), false, true ));
            assert(d1(c) ==   std::make_tuple(true, std::tie(typeid(B)), true,  true ));
            assert(d1(V{}) == std::make_tuple(true, std::tie(typeid(A)), false, false));

            assert(d2(v) ==   std::make_tuple(true,  std::tie(typeid(A)), false, true ));
            assert(d2(c) ==   std::make_tuple(true,  std::tie(typeid(B)), true,  true ));
            assert(d2(V{}) == std::make_tuple(true,  std::tie(typeid(A)), false, false));

            assert(d3(v) ==   std::make_tuple(true,  std::tie(typeid(A)), false, true ));
            assert(d3(c) ==   std::make_tuple(true,  std::tie(typeid(B)), true,  true ));
            assert(d3(V{}) == std::make_tuple(true,  std::tie(typeid(A)), false, false));

            assert(d4(v) ==   std::make_tuple(false, std::tie(typeid(A)), false, true ));
            assert(d4(c) ==   std::make_tuple(false, std::tie(typeid(B)), true,  true ));
            assert(d4(V{}) == std::make_tuple(false, std::tie(typeid(A)), false, false));

            assert(d5(v) ==   std::make_tuple(true,  std::tie(typeid(A)), false, true ));
            assert(d5(c) ==   std::make_tuple(true,  std::tie(typeid(B)), true,  true ));
            assert(d5(V{}) == std::make_tuple(true,  std::tie(typeid(A)), false, false));
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

            assert(d(v)                    == std::make_tuple(false, true,  false, true ));
            assert(cd(v)                   == std::make_tuple(true,  true,  false, true ));
            assert(visit(v_)(v)            == std::make_tuple(false, false, false, true ));

            assert(dcv(v)                  == std::make_tuple(true,  true,  false, true ));
            assert(cdcv(v)                 == std::make_tuple(true,  true,  false, true ));
            assert(visit(cv_)(v)           == std::make_tuple(true,  false, false, true ));

            assert(dmv(v)                  == std::make_tuple(false, true,  false, true ));
            assert(cdmv(v)                 == std::make_tuple(true,  true,  false, true ));
            assert(visit(visitor3{})(v)    == std::make_tuple(false, false, false, true ));

            assert(d(cv)                   == std::make_tuple(false, true,  true,  true ));
            assert(cd(cv)                  == std::make_tuple(true,  true,  true,  true ));
            assert(visit(v_)(cv)           == std::make_tuple(false, false, true,  true ));

            assert(dcv(cv)                 == std::make_tuple(true,  true,  true,  true ));
            assert(cdcv(cv)                == std::make_tuple(true,  true,  true,  true ));
            assert(visit(cv_)(cv)          == std::make_tuple(true,  false, true,  true ));

            assert(dmv(cv)                 == std::make_tuple(false, true,  true,  true ));
            assert(cdmv(cv)                == std::make_tuple(true,  true,  true,  true ));
            assert(visit(visitor3{})(cv)   == std::make_tuple(false, false, true,  true ));

            assert(d(V{})                  == std::make_tuple(false, true,  false, false));
            assert(cd(V{})                 == std::make_tuple(true,  true,  false, false));
            assert(visit(v_)(V{})          == std::make_tuple(false, false, false, false));

            assert(dcv(V{})                == std::make_tuple(true,  true,  false, false));
            assert(cdcv(V{})               == std::make_tuple(true,  true,  false, false));
            assert(visit(cv_)(V{})         == std::make_tuple(true,  false, false, false));

            assert(dmv(V{})                == std::make_tuple(false, true,  false, false));
            assert(cdmv(V{})               == std::make_tuple(true,  true,  false, false));
            assert(visit(visitor3{})(V{})  == std::make_tuple(false, false, false, false));
        }
        {
            using V = variant< RW< int >, double >;
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
        { // invoke
            struct A { int operator () (int) { return 0; } bool operator () (double) { return true; } };
            struct B { int operator () (int) { return 1; } bool operator () (double) { return false; } };
            using V = variant< A, B >;
            V v;
            static_assert(std::is_same< int, decltype(invoke(v, 0)) >{});
            static_assert(std::is_same< bool, decltype(invoke(v, 0.0)) >{});
            assert(invoke(v, 0) == 0);
            assert(invoke(v, 1.0) == true);
            v = B{};
            assert(invoke(v, 0) == 1);
            assert(invoke(v, 1.0) == false);
        }
        { // invoke
            auto a = [] (auto &&... _values) -> int { return +static_cast< int >(sizeof...(_values)); };
            auto b = [] (auto &&... _values) -> int { return -static_cast< int >(sizeof...(_values)); };
            using V = variant< decltype(a), decltype(b) >;
            V v = a;
            static_assert(std::is_same< int, decltype(invoke(v, 0)) >{});
            assert(invoke(v, 0, 1.0f, 2.0, 3.0L, true) == +5);
            assert(invoke(v) == 0);
            assert(invoke(v, nullptr) == 1);
            v = std::move(b);
            assert(invoke(v, 0, 1.0f, 2.0, 3.0L, true) == -5);
            assert(invoke(v) == 0);
            assert(invoke(v, nullptr) == -1);
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
                assert(0 == visit(l, c));
                assert(1 == visit(l, v));
                assert(2 == visit(l, std::move(c)));
                assert(3 == visit(l, std::move(v)));
                assert(3 == visit(l, std::move(v)));
                static_assert(std::is_void< decltype(visit(l)()) >{});
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
                    assert(0 == l(ac));
                    assert(1 == l(av));
                    assert(2 == l(std::move(ac)));
                    assert(3 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l1, l2, l4);
                    assert(0 == l(ac));
                    assert(1 == l(av));
                    assert(2 == l(std::move(ac)));
                    assert(4 == l(std::move(av)));
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
                    auto const l = compose_visitors(l0, l1, l4);
                    assert(0 == l(ac));
                    assert(1 == l(av));
                    assert(4 == l(std::move(ac)));
                    assert(4 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l2, l3);
                    assert(0 == l(ac));
                    assert(3 == l(av));
                    assert(2 == l(std::move(ac)));
                    assert(3 == l(std::move(av)));
                }
                {
                    auto const l = compose_visitors(l0, l2, l4);
                    assert(0 == l(ac));
                    assert(0 == l(av));
                    assert(2 == l(std::move(ac)));
                    assert(4 == l(std::move(av)));
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
                using V = variant< RW< A > >;
                V a;
                V const c{};
                {
                    auto const la = compose_visitors([] (A const &) { return -1; }, l);
                    assert(-1 == visit(la, c));
                    assert(1 == la(a));
                    assert(2 == la(std::move(c)));
                    assert(3 == la(A{}));
                }
                {
                    struct F { auto operator () (A const &) && { return -11; } };
                    auto lam = compose_visitors(F{}, l);
                    assert(0 == lam(c));
                    assert(-11 == visit(std::move(lam), c));
                }
            }
        }
        { // function pointers
            using V = variant< decltype(&f), decltype(&g) >;
            V v = g;
            assert(v.active< decltype(&f) >());
            assert(static_cast< decltype(&f) >(v) == &g);
            assert(invoke(v) == 2);
            v = f;
            assert(v.active< decltype(&g) >());
            assert(static_cast< decltype(&g) >(v) == &f);
            assert(invoke(v) == 1);
            auto l = [] { return 323; };
            v = static_cast< decltype(&g) >(l);
            assert(invoke(v) == 323);
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

        assert(multivisit(v, U{a}, V{a}) == 0);
        assert(multivisit(v, U{a}, V{b}) == 1);
        assert(multivisit(v, U{b}, V{a}) == 2);
        assert(multivisit(v, U{b}, V{b}) == 3);

        assert(multivisit(v, V{a}, U{a}) == 0);
        assert(multivisit(v, V{a}, U{b}) == 1);
        assert(multivisit(v, V{b}, U{a}) == 2);
        assert(multivisit(v, V{b}, U{b}) == 3);
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
        using V = boost_variant_i< boost::recursive_wrapper< A >, B >;
        V a{A{}};
        assert(a.active< A >());
        assert(a.active< boost::recursive_wrapper< A > >());
        V b{B{}};
        assert(b.active< B >());
        assert(multivisit(v, a, a) == 0);
        assert(multivisit(v, a, b) == 1);
        assert(multivisit(v, b, a) == 2);
        assert(multivisit(v, b, b) == 3);
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
        using V = boost_variant_c< A, boost::recursive_wrapper< B > >;
        V a{A{}};
        assert(a.active< A >());
        V b{B{}};
        assert(b.active< B >());
        assert(b.active< boost::recursive_wrapper< B > >());
        assert(multivisit(v, a, a) == 0);
        assert(multivisit(v, a, b) == 1);
        assert(multivisit(v, b, a) == 2);
        assert(multivisit(v, b, b) == 3);
    }
    { // handling of the empty
        versatile<> empty;
        using V = versatile< int, double, versatile<> >;
        V v;
        auto l = compose_visitors([] (int) { return 0; }, [] (double) { return 1; }, [] (auto x) { static_assert(std::is_same< decltype(x), versatile<> >{}); return 2; });
        assert(0 == multivisit(l, int{}));
        assert(1 == multivisit(l, double{}));
        assert(2 == multivisit(l, empty));
        assert(0 == multivisit(l, v));
        assert(1 == multivisit(l, V{double{}}));
        assert(0 == multivisit(l, V{empty})); // default construction equivalent
    }
    { // aggregate wrapper
        struct X {};
        struct Y {};

        struct XY { X x; Y y; XY() = delete; XY(XY const &) = delete; XY(XY &&) = default; XY & operator = (XY &&) = default; };
        XY xy{}; // value-initialization by empty list initializer
        xy.x = {}; xy.y = {}; // accessible
        xy = XY{};
        static_assert(std::is_constructible< XY, XY >{});
        static_assert(!std::is_copy_constructible< XY >{});
        static_assert(std::is_move_constructible< XY >{});
        static_assert(!std::is_default_constructible< XY >{});

        using WXY = AW< XY >;
        static_assert(std::is_assignable< WXY &, XY >{});
        static_assert(std::is_constructible< WXY, XY >{});
        static_assert(!std::is_copy_constructible< WXY >{}); // mimic
        static_assert(std::is_move_constructible< WXY >{});
        static_assert(!std::is_default_constructible< WXY >{});
        WXY wxy{std::move(xy)};
        wxy = XY{};
        wxy = WXY{XY{}};

        using V = variant< WXY >;
        static_assert(std::is_assignable< V &, XY >{});
        static_assert(std::is_assignable< V &, WXY >{});
        static_assert(std::is_constructible< V, XY >{});
        static_assert(std::is_copy_constructible< V >{}); // lie
        static_assert(std::is_move_constructible< V >{});
        static_assert(std::is_default_constructible< V >{});
        V v{in_place, X{}, Y{}};
        assert(v.active< XY >());
        assert(v.active< WXY >());
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
        assert(v.active< XY >());
        v.replace(X{});
        assert(v.active< X >());
        v.replace(Y{});
        assert(v.active< Y >());
        v.emplace(X{});
        assert(v.active< XY >());
        v.emplace(Y{});
        assert(v.active< Y >());
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
        static_assert(std::is_literal_type< V >{});

        struct visitor3
        {
            constexpr auto operator () (A, int i = 1) const { return 100 + i; }
            constexpr auto operator () (B, int i = 2) const { return 200 + i; }
            constexpr auto operator () (C, int i = 3) const { return 300 + i; }
        };
        static_assert(std::is_literal_type< visitor3 >{});

        // rrefs
        static_assert(visit(visitor3{}, V{A{}}) == 101);
        static_assert(visit(visitor3{}, V{B{}}) == 202);
        static_assert(visit(visitor3{}, V{C{}}) == 303);

        static_assert(visit(visitor3{}, V{A{}}, 10) == 110);
        static_assert(visit(visitor3{}, V{B{}}, 20) == 220);
        static_assert(visit(visitor3{}, V{C{}}, 30) == 330);

        // lrefs
        constexpr visitor3 visitor3_{};
        constexpr V a{A{}};
        constexpr V b{B{}};
        constexpr V c{C{}};

        static_assert(visit(visitor3_, a) == 101);
        static_assert(visit(visitor3_, b) == 202);
        static_assert(visit(visitor3_, c) == 303);

        static_assert(visit(visitor3_, a, 11) == 111);
        static_assert(visit(visitor3_, b, 22) == 222);
        static_assert(visit(visitor3_, c, 33) == 333);

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
        static_assert(std::is_literal_type< multivisitor3 >{});

        constexpr auto multivisitor3_ = visit(multivisitor3{});
        static_assert(multivisitor3_(a,   0, a) == 111);
        static_assert(multivisitor3_(a,   0, b) == 112);
        static_assert(multivisitor3_(a,   0, c) == 113);
        static_assert(multivisitor3_(b,   0, a) == 121);
        static_assert(multivisitor3_(b,   0, b) == 122);
        static_assert(multivisitor3_(b,   0, c) == 123);
        static_assert(multivisitor3_(c,   0, a) == 131);
        static_assert(multivisitor3_(c,   0, b) == 132);
        static_assert(multivisitor3_(c,   0, c) == 133);
        static_assert(multivisitor3_(a, 0.0, a) == 211);
        static_assert(multivisitor3_(a, 0.0, b) == 212);
        static_assert(multivisitor3_(a, 0.0, c) == 213);
        static_assert(multivisitor3_(b, 0.0, a) == 221);
        static_assert(multivisitor3_(b, 0.0, b) == 222);
        static_assert(multivisitor3_(b, 0.0, c) == 223);
        static_assert(multivisitor3_(c, 0.0, a) == 231);
        static_assert(multivisitor3_(c, 0.0, b) == 232);
        static_assert(multivisitor3_(c, 0.0, c) == 233);
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
    { // compile time wasteful test
        // -ftemplate-depth=32:
        static_assert((test_perferct_forwarding< cvariant, 2, 3 >{}())); // up to 2, 10 can be compiled too
        // -ftemplate-depth=41:
        static_assert((test_perferct_forwarding< cvariant, 3, 2 >{}())); // 3 is maximum multivisitor arity for test which can be compiled
        // 3, 3 and 4, 2 is too hard for clang++ (hard "error: constexpr evaluation hit maximum step limit")
    }
    {
        assert((test_perferct_forwarding< variant, 2, 6 >{}()));
    }
    { // -ftemplate-depth=40 for 5, 5
        assert((hard< ROWS, COLS >()));
    }
    return EXIT_SUCCESS;
}
#endif
