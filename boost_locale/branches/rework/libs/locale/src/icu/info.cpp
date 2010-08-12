//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_LOCALE_SOURCE
#include <boost/locale/info.hpp>
#include "cdata.hpp"
#include "all_generator.hpp"


namespace boost {
    namespace locale {
        namespace impl_icu {

            class icu_info : public info {
            public:
                icu_info(cdata const &d, size_t refs = 0) : 
                    info(refs)
                {
                    language_ = d.locale.getLanguage();
                    country_ = d.locale.getCountry();
                    variant_ = d.locale.getVariant();
                    encoding_ = d.encoding;
                    utf8_ = d.utf8;
                }

                virtual std::string get_string_property(string_propery v) const
                {
                    switch(v) {
                    case language_property:
                        return language_;
                    case country_property:
                        return country_;
                    case variant_property:
                        return variant_;
                    case encoding_property:
                        return encoding_;
                    default:
                        return "";
                    };
                }

                virtual int get_ineger_property(integer_property v) const
                {
                    switch(v) {
                    case utf8_property:
                        return utf8_;
                    default:
                        return 0;
                    }
                }
            private:
                std::string language_;
                std::string country_;
                std::string variant_;
                std::string encoding_;
                bool utf8_;
            };

            std::locale create_info(std::locale const &in,cdata const &d)
            {
                return std::locale(in, new icu_info(d));
            }


        } // impl_icu

    } // locale
} //boost
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
