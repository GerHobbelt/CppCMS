#ifndef BOOST_LOCALE_SRC_ICU_IMPL_HPP_INCLUDED
#define BOOST_LOCALE_SRC_ICU_IMPL_HPP_INCLUDED

#include <string>
#include <unicode/locid.h>
namespace boost {
    namespace locale {
        struct info_impl {
            icu::Locale locale;
            std::string encoding;
        };
    }
}

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

