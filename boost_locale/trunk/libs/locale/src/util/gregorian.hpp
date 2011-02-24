//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_SRC_UTIL_GREGORIAN_HPP
#define BOOST_LOCALE_SRC_UTIL_GREGORIAN_HPP

#include <boost/locale/date_time_facet.hpp>

namespace boost {
namespace locale {
namespace util {

    abstract_calendar *create_gregorian_calendar(std::string const &terr);

} // util
} // locale 
} //boost


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
