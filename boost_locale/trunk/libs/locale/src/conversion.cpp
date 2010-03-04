//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_LOCALE_SOURCE
#include <boost/locale/conversion.hpp>
#include <boost/locale/info.hpp>
#include <unicode/normlzr.h>
#include <unicode/ustring.h>

#include "info_impl.hpp"
#include "uconv.hpp"


namespace boost {
namespace locale {
namespace impl {

void do_normalize(icu::UnicodeString &str,int flags)
{
    UErrorCode code=U_ZERO_ERROR;
    UNormalizationMode mode=UNORM_DEFAULT;
    switch(flags) {
    case norm_nfd:
        mode=UNORM_NFD;
        break;
    case norm_nfc:
        mode=UNORM_NFC;
        break;
    case norm_nfkd:
        mode=UNORM_NFKD;
        break;
    case norm_nfkc:
        mode=UNORM_NFKC;
        break;
    }
    icu::UnicodeString tmp;
    icu::Normalizer::normalize(str,mode,0,tmp,code);

    impl::check_and_throw_icu_error(code);

    str=tmp;
}

template<typename CharType>
std::basic_string<CharType> do_convert(conversion_type how,CharType const *begin,CharType const *end,int flags,std::locale const *loc)
{
    if(how == normalization) {
        icu_std_converter<CharType> cvt("UTF-8");
        icu::UnicodeString tmp=cvt.icu(begin,end);
        do_normalize(tmp,flags);
        return cvt.std(tmp);
    }
    else {
        info const &inf=std::use_facet<info>(*loc);
        icu_std_converter<CharType> cvt(inf.encoding());
        icu::Locale const &locale=inf.impl()->locale;
        icu::UnicodeString str=cvt.icu(begin,end);
        switch(how) {
        case upper_case:
            str.toUpper(locale);
            break;
        case lower_case:
            str.toLower(locale);
            break;
        case title_case:
            str.toTitle(0,locale);
            break;
        case case_folding:
            str.foldCase();
            break;
        default:
            ;
        }
        return cvt.std(str);
    }

}

BOOST_LOCALE_DECL std::string convert(conversion_type how,char const *begin,char const *end,int flags,std::locale const *loc)
{
    return do_convert(how,begin,end,flags,loc);
}

#ifndef BOOST_NO_STD_WSTRING
BOOST_LOCALE_DECL std::wstring convert(conversion_type how,wchar_t const *begin,wchar_t const *end,int flags,std::locale const *loc)
{
    return do_convert(how,begin,end,flags,loc);
}
#endif

#ifdef BOOST_HAS_CHAR16_T
BOOST_LOCALE_DECL std::u16string convert(conversion_type how,char16_t const *begin,char16_t const *end,int flags,std::locale const *loc)
{
    return do_convert(how,begin,end,flags,loc);
}
#endif

#ifdef BOOST_HAS_CHAR32_T
BOOST_LOCALE_DECL std::u32string convert(conversion_type how,char32_t const *begin,char32_t const *end,int flags,std::locale const *loc)
{
    return do_convert(how,begin,end,flags,loc);
}
#endif
} // namespace impl
} // locale
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
