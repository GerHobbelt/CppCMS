//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_ICU_CDATA_HPP
#define BOOST_LOCALE_ICU_CDATA_HPP

namespace boost {
    namespace locale {
        namespace impl_icu {
            std::locale create_convert(std::locale const &,cdata const &,character_facet_type);
            std::locale create_collate(std::locale const &,cdata const &,character_facet_type);
            std::locale create_formatting(std::locale const &,cdata const &,character_facet_type);
            std::locale create_parsing(std::locale const &,cdata const &,character_facet_type);
            std::locale create_message(std::locale const &,cdata const &,character_facet_type,
                        std::vector<std::string> const &domains,std::vector<std::string> const &paths);
            std::locale create_boundary(std::locale const &,cdata const &,character_facet_type);
            std::locale create_calendar(std::locale const &,cdata const &);
            std::locale create_info(std::locale const &,cdata const &);

        }
    }
}

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
