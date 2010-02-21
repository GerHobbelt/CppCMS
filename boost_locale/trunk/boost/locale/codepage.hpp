//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_CODEPAGE_HPP_INCLUDED
#define BOOST_LOCALE_CODEPAGE_HPP_INCLUDED

#include <boost/locale/config.hpp>
#include <boost/locale/info.hpp>
#include <boost/cstdint.hpp>
#include <stdexcept>

namespace boost {
    namespace locale {

        template<typename CharType>
        std::codecvt<CharType,char,mbstate_t> *create_codecvt(info const &inf);
        
        template<>
        BOOST_LOCALE_DECL std::codecvt<char,char,mbstate_t> *create_codecvt(info const &inf);

        #ifndef BOOST_NO_STD_WSTRING
        template<>
        BOOST_LOCALE_DECL std::codecvt<wchar_t,char,mbstate_t> *create_codecvt(info const &inf);
        #endif

        #ifdef BOOST_HAS_CHAR16_T
        template<>
        BOOST_LOCALE_DECL std::codecvt<char16_t,char,mbstate_t> *create_codecvt(info const &inf);
        #endif

        #ifdef BOOST_HAS_CHAR32_T
        template<>
        BOOST_LOCALE_DECL std::codecvt<char32_t,char,mbstate_t> *create_codecvt(info const &inf);
        #endif

        namespace conv {
            class conversion_error : public std::runtime_error {
            public:
                conversion_error() : std::runtime_error("Conversion failed") {}
            };

            typedef enum {
                skip            = 0, ///!< Skip illegal/unconvertable characters
                stop            = 1, ///!< Stop conversion and throw conversion_error
                default_method  = skip ///!< Default method - skip
            } method_type;

            template<typename CharType>
            std::basic_string<CharType> to_utf(char const *begin,char const *end,std::string const &charset,method_type how=default_method);

            template<typename CharType>
            std::string from_utf(CharType const *begin,CharType const *end,std::string const &charset,method_type how=default_method);

            template<typename CharType>
            std::basic_string<CharType> to_utf(char const *begin,char const *end,std::locale const &loc,method_type how=default_method)
            {
                return to_utf<CharType>(begin,end,std::use_facet<info>(loc).encoding(),how);
            }

            template<typename CharType>
            std::string from_utf(CharType const *begin,CharType const *end,std::locale const &loc,method_type how=default_method)
            {
                return from_utf(begin,end,std::use_facet<info>(loc).encoding(),how);
            }

            template<typename CharType>
            std::basic_string<CharType> to_utf(std::string const &text,std::string const &charset,method_type how=default_method)
            {
                return to_utf<CharType>(text.c_str(),text.c_str()+text.size(),charset,how);
            }

            template<typename CharType>
            std::string from_utf(std::basic_string<CharType> const &text,std::string const &charset,method_type how=default_method)
            {
                return from_utf(text.c_str(),text.c_str()+text.size(),charset,how);
            }

            template<typename CharType>
            std::basic_string<CharType> to_utf(char const *text,std::string const &charset,method_type how=default_method)
            {
                char const *text_end = text;
                while(*text_end) 
                    text_end++;
                return to_utf<CharType>(text,text_end,charset,how);
            }

            template<typename CharType>
            std::string from_utf(CharType const *text,std::string const &charset,method_type how=default_method)
            {
                CharType const *text_end = text;
                while(*text_end) 
                    text_end++;
                return from_utf(text,text_end,charset,how);
            }

            template<typename CharType>
            std::basic_string<CharType> to_utf(std::string const &text,std::locale const &loc,method_type how=default_method)
            {
                return to_utf<CharType>(text.c_str(),text.c_str()+text.size(),loc,how);
            }

            template<typename CharType>
            std::string from_utf(std::basic_string<CharType> const &text,std::locale const &loc,method_type how=default_method)
            {
                return from_utf(text.c_str(),text.c_str()+text.size(),loc,how);
            }

            template<typename CharType>
            std::basic_string<CharType> to_utf(char const *text,std::locale const &loc,method_type how=default_method)
            {
                char const *text_end = text;
                while(*text_end) 
                    text_end++;
                return to_utf<CharType>(text,text_end,loc,how);
            }

            template<typename CharType>
            std::string from_utf(CharType const *text,std::locale const &loc,method_type how=default_method)
            {
                CharType const *text_end = text;
                while(*text_end) 
                    text_end++;
                return from_utf(text,text_end,loc,how);
            }

            template<>
            BOOST_LOCALE_DECL std::basic_string<char> to_utf(char const *begin,char const *end,std::string const &charset,method_type how);

            template<>
            BOOST_LOCALE_DECL std::string from_utf(char const *begin,char const *end,std::string const &charset,method_type how);

            #ifndef BOOST_NO_STD_WSTRING
            template<>
            BOOST_LOCALE_DECL std::basic_string<wchar_t> to_utf(char const *begin,char const *end,std::string const &charset,method_type how);

            template<>
            BOOST_LOCALE_DECL std::string from_utf(wchar_t const *begin,wchar_t const *end,std::string const &charset,method_type how);
            #endif

            #ifdef BOOST_HAS_CHAR16_T
            template<>
            BOOST_LOCALE_DECL std::basic_string<char16_t> to_utf(char const *begin,char const *end,std::string const &charset,method_type how);

            template<>
            BOOST_LOCALE_DECL std::string from_utf(char16_t const *begin,char16_t const *end,std::string const &charset,method_type how);
            #endif

            #ifdef BOOST_HAS_CHAR32_T
            template<>
            BOOST_LOCALE_DECL std::basic_string<char32_t> to_utf(char const *begin,char const *end,std::string const &charset,method_type how);

            template<>
            BOOST_LOCALE_DECL std::string from_utf(char32_t const *begin,char32_t const *end,std::string const &charset,method_type how);
            #endif
        } // conv

    } // locale
} // boost


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

