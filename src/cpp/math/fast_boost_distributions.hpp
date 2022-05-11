#pragma once

#include <boost/math/distributions.hpp>

namespace fast_boost {
    using namespace boost::math::policies;
    typedef policy<promote_double<false>> fast_policy;

    BOOST_MATH_DECLARE_DISTRIBUTIONS(float, fast_policy)
}
