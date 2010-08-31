//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_UTIL_INFO_HPP
#define BOOST_LOCALE_UTIL_INFO_HPP

#include <locale>

namespace boost {
    namespace locale {
        namespace util {
            std::locale create_info(std::locale const &,std::string const &name); // ok
        }
    }
}

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
