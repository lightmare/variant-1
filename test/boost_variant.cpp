#include "boost_variant.hpp"
#include "visit.hpp"

#include <cstdlib>

int
main()
{
    using ::versatile::identity;
    using ::test::common_type;
    using ::test::perferct_forwarding;
    { // boost::variant
        {
            struct L {};
            SA(std::is_literal_type< L >{});
            SA(!std::is_literal_type< ::boost::variant< L > >{});
        }
        using ::test_boost_variant::boost_variant_i;
        using ::test_boost_variant::boost_variant_c;
        using ::test_boost_variant::boost_recursive_wrapper;
        {
            assert ((perferct_forwarding< common_type, boost_variant_i, identity,                2, 2 >::run()));
            assert ((perferct_forwarding< common_type, boost_variant_i, boost_recursive_wrapper, 2, 2 >::run()));
        }
        {
            assert ((perferct_forwarding< common_type, boost_variant_c, identity,                2, 2 >::run()));
            assert ((perferct_forwarding< common_type, boost_variant_c, boost_recursive_wrapper, 2, 2 >::run()));
        }
    }
    return EXIT_SUCCESS;
}
