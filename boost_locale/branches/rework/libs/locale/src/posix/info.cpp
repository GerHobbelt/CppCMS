//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_LOCALE_SOURCE
#include <locale>
#include <string>
#include <ios>
#include <boost/locale/generator.hpp>
#include <boost/locale/info.hpp>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "all_generator.hpp"

#include "../std/locale_data.hpp"

namespace boost {
namespace locale {
namespace impl_posix {
    using boost::locale::impl_std::locale_data;
    class posix_info : public info {
    public:
        posix_info(std::string const &name,size_t refs = 0) :
            info(refs)
        {
            std::string rname;

            if(name.empty()) {
                char const *pname = setlocale(LC_CTYPE,NULL);
                if(pname && (strcmp(pname,"C")==0 || strcmp(pname,"POSIX")==0))
                    pname = 0;
                if(!pname) {
                    pname = getenv("LC_CTYPE");
                }
                if(!pname) {
                    pname = getenv("LC_ALL");
                }
                if(!pname) {
                    pname = getenv("LANG");
                }
                if(!pname) {
                    pname = "C";
                }
                rname = pname;
            }
            else 
                rname = name;

            d.parse(rname);
        }
        virtual std::string get_string_property(string_propery v) const
        {
            switch(v) {
            case language_property:
                return d.language;
            case country_property:
                return d.country;
            case variant_property:
                return d.variant;
            case encoding_property:
                return d.encoding;
            default:
                return "";
            };
        }

        virtual int get_ineger_property(integer_property v) const
        {
            switch(v) {
            case utf8_property:
                return d.utf8;
            default:
                return 0;
            }
        }
    private:
        locale_data d;
    };
    
    std::locale create_info(std::locale const &in,std::string const &name)
    {
        return std::locale(in,new posix_info(name));
    }


} // impl_std
} // locale 
} //boost



// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
