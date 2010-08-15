//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_IMPL_STD_WINDOWS_LOCALE_NAME_HPP
#define BOOST_LOCALE_IMPL_STD_WINDOWS_LOCALE_NAME_HPP

#include <string>

namespace boost {
    namespace locale {
        namespace impl_std {
            std::string get_windows_country_by_code(std::string const &key);
            std::string get_windows_language_by_code(std::string const &key);
        }
    }
}

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
