//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_IMPL_POSIX_CODECVT_HPP
#define BOOST_LOCALE_IMPL_ICU_CODECVT_HPP
#include <boost/locale/config.hpp>
#include "../util/codecvt_converter.hpp"
namespace boost {
namespace locale {
namespace impl_icu {
    BOOST_LOCALE_DECL
    std::auto_ptr<util::base_converter> create_uconv_converter(std::string const &encoding);

} // impl_icu
} // locale 
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
