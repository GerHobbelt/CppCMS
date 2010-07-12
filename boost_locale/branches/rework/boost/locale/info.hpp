//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_INFO_HPP_INCLUDED
#define BOOST_LOCALE_INFO_HPP_INCLUDED
#include <boost/locale/config.hpp>
#ifdef BOOST_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4275 4251 4231 4660)
#endif
#include <locale>
#include <string>
#include <map>
#include <memory>


namespace boost {
    namespace locale {

        ///
        /// \brief a facet that holds general information about locale
        ///
        /// This facet should be always created in order to make all Boost.Locale functions work
        ///
        class BOOST_LOCALE_DECL info : public std::locale::facet
        {
        public:
            static std::locale::id id; ///< This member defines uniquely this facet, required by STL 
           
            info(size_t refs = 0) : std::locale::facet(refs)
            {
            }
            ///
            /// Get language name
            ///
            virtual std::string language() const = 0;
            ///
            /// Get country name
            ///
            virtual std::string country() const = 0;
            ///
            /// Get locale variant
            ///
            virtual std::string variant() const = 0;
            ///
            /// Get encoding
            ///
            virtual std::string encoding() const = 0;

            ///
            /// Is underlying encoding is UTF-8 (for char streams and strings)
            ///
            virtual bool utf8() const = 0;
            
#if defined (__SUNPRO_CC) && defined (_RWSTD_VER)
            std::locale::id& __get_id (void) const { return id; }
#endif
        };

    }
}

#ifdef BOOST_MSVC
#pragma warning(pop)
#endif

#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
