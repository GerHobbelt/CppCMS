//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_IMPL_ICU_CODECVT_HPP
#define BOOST_LOCALE_IMPL_ICU_CODECVT_HPP
namespace boost {
namespace locale {
namespace impl_icu {
    
    template<typename CharType>
    std::codecvt<CharType,char,mbstate_t> *create_codecvt(std::string const &encoding);

    template<>
    BOOST_LOCALE_DECL std::codecvt<char,char,mbstate_t> *create_codecvt(std::string const &encoding);

    #ifndef BOOST_NO_STD_WSTRING
    template<>
    BOOST_LOCALE_DECL std::codecvt<wchar_t,char,mbstate_t> *create_codecvt(std::string const &encoding);
    #endif

    #ifdef BOOST_HAS_CHAR16_T
    template<>
    BOOST_LOCALE_DECL std::codecvt<char16_t,char,mbstate_t> *create_codecvt(std::string const &encoding);
    #endif

    #ifdef BOOST_HAS_CHAR32_T
    template<>
    BOOST_LOCALE_DECL std::codecvt<char32_t,char,mbstate_t> *create_codecvt(std::string const &encoding);
    #endif

} // impl_icu
} // locale 
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
#endif
