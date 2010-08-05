//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_LOCALE_SOURCE
#include <boost/locale/codepage.hpp>
#include "../icu/uconv.hpp"
#include <unicode/ucnv.h>
#include <unicode/ucnv_err.h>

#ifdef BOOST_MSVC
#  pragma warning(disable : 4244) // loose data 
#endif

#include "icu_util.hpp"
#include <vector>
namespace boost {
namespace locale {
    namespace conv {
        template<typename CharType>
        std::basic_string<CharType> to_utf_impl(char const *begin,char const *end,std::string const &charset,method_type how=default_method)
        {
            impl_icu::icu_std_converter<char> cvt_from(charset,how == skip ? impl_icu::cvt_skip : impl_icu::cvt_stop);
            impl_icu::icu_std_converter<CharType> cvt_to("UTF-8",how == skip ? impl_icu::cvt_skip : impl_icu::cvt_stop);
            try {
                return cvt_to.std(cvt_from.icu(begin,end));
            }
            catch(std::exception const &/*e*/) {
                throw conversion_error();
            }
        }

        template<typename CharType>
        std::string from_utf_impl(CharType const *begin,CharType const *end,std::string const &charset,method_type how=default_method)
        {
            impl_icu::icu_std_converter<CharType> cvt_from("UTF-8",how == skip ? impl_icu::cvt_skip : impl_icu::cvt_stop);
            impl_icu::icu_std_converter<char> cvt_to(charset,how == skip ? impl_icu::cvt_skip : impl_icu::cvt_stop);
            try {
                return cvt_to.std(cvt_from.icu(begin,end));
            }
            catch(std::exception const &/*e*/) {
                throw conversion_error();
            }
        }

        template<>
        BOOST_LOCALE_DECL std::basic_string<char> to_utf(char const *begin,char const *end,std::string const &charset,method_type how)
        {
            return to_utf_impl<char>(begin,end,charset,how);
        }

        template<>
        BOOST_LOCALE_DECL std::string from_utf(char const *begin,char const *end,std::string const &charset,method_type how)
        {
            return from_utf_impl<char>(begin,end,charset,how);
        }

        #if !defined(BOOST_NO_STD_WSTRING) || defined(BOOST_WINDOWS)
        template<>
        BOOST_LOCALE_DECL std::basic_string<wchar_t> to_utf(char const *begin,char const *end,std::string const &charset,method_type how)
        {
            return to_utf_impl<wchar_t>(begin,end,charset,how);
        }

        template<>
        BOOST_LOCALE_DECL std::string from_utf(wchar_t const *begin,wchar_t const *end,std::string const &charset,method_type how)
        {
            return from_utf_impl<wchar_t>(begin,end,charset,how);
        }
        #endif

        #ifdef BOOST_HAS_CHAR16_T
        template<>
        BOOST_LOCALE_DECL std::basic_string<char16_t> to_utf(char const *begin,char const *end,std::string const &charset,method_type how)
        {
            return to_utf_impl<char16_t>(begin,end,charset,how);
        }


        template<>
        BOOST_LOCALE_DECL std::string from_utf(char16_t const *begin,char16_t const *end,std::string const &charset,method_type how)
        {
            return from_utf_impl<char16_t>(begin,end,charset,how);
        }
        #endif

        #ifdef BOOST_HAS_CHAR32_T
        template<>
        BOOST_LOCALE_DECL std::basic_string<char32_t> to_utf(char const *begin,char const *end,std::string const &charset,method_type how)
        {
            return to_utf_impl<char32_t>(begin,end,charset,how);
        }

        template<>
        BOOST_LOCALE_DECL std::string from_utf(char32_t const *begin,char32_t const *end,std::string const &charset,method_type how)
        {
            return from_utf_impl<char32_t>(begin,end,charset,how);
        }
        #endif
    } // conv

} // locale 
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
