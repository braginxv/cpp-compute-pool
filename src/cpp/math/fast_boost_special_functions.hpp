#pragma once

#include <boost/math/special_functions.hpp>

namespace fast_boost {
    using namespace boost::math::policies;
    typedef policy<promote_double<false>> fast_policy;

    BOOST_MATH_DECLARE_SPECIAL_FUNCTIONS(fast_policy)
}
