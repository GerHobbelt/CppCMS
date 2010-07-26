//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_LOCALE_SOURCE
#include <boost/locale/conversion.hpp>
#include <boost/locale/generator.hpp>
#include <unicode/normlzr.h>
#include <unicode/ustring.h>

#include "cdata.hpp"
#include "uconv.hpp"


namespace boost {
namespace locale {
namespace impl_icu {
    template<typename CharType>
    class converter_impl : public converter<CharType> {
    public:
        typedef CharType char_type;
        typedef std::basic_string<char_type> string_type;

        converter_impl(cdata const &d) :
            locale_(d.locale),
            encoding_(d.encoding)
        {
        }


        virtual string_type convert(converter_base::conversion_type how,char_type const *begin,char_type const *end,int flags = 0) const
        {
            icu_std_converter<char_type> cvt(encoding_);
            icu::UnicodeString str=cvt.icu(begin,end);
            switch(how) {
            case converter_base::normalization:
                do_normalize(str,flags);
                break;
            case converter_base::upper_case:
                str.toUpper(locale_);
                break;
            case converter_base::lower_case:
                str.toLower(locale_);
                break;
            case converter_base::title_case:
                str.toTitle(0,locale_);
                break;
            case converter_base::case_folding:
                str.foldCase();
                break;
            default:
                ;
            }
            return cvt.std(str);
        }
    
    private:

        void do_normalize(icu::UnicodeString &str,int flags) const
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

            check_and_throw_icu_error(code);

            str=tmp;
        }

        icu::Locale locale_;
        std::string encoding_;
    }; // converter_impl

    /*
    class utf8_converter_impl : public converter<char> {
    public:
        
        converter_impl(cdata const &d) :
            locale_(d.locale),
        {
        }


        virtual string_type convert(converter_base::conversion_type how,char const *begin,cha const *end,int flags = 0) const
        {
            
            if(how == converter_base::normalization)
                return do_normalize(begin,end,flags);

            std::vector<char> buf((end-begin+1)*11/10,0);
            UCaseMap *csm= // TODO
            case converter_base::upper_case:
                str.toUpper(locale_);
                break;
            case converter_base::lower_case:
                str.toLower(locale_);
                break;
            case converter_base::title_case:
                str.toTitle(0,locale_);
                break;
            case converter_base::case_folding:
                str.foldCase();
                break;
            default:
                ;
            }
            return cvt.std(str);
        }
    
    private:

        icu::Locale locale_;
    }; // converter_impl
*/


    std::locale create_convert(std::locale const &in,cdata const &cd,character_facet_type type)
    {
        switch(type) {
        case char_facet:
            return std::locale(in,new converter_impl<char>(cd));
        #ifndef BOOST_NO_STD_WSTRING
        case wchar_t_facet:
            return std::locale(in,new converter_impl<wchar_t>(cd));
        #endif
        #ifdef BOOST_HAS_CHAR16_T
        case char16_t_facet:
            return std::locale(in,new converter_impl<char16_t>(cd));
        #endif
        #ifdef BOOST_HAS_CHAR32_T
        case char32_t_facet:
            return std::locale(in,new converter_impl<char32_t>(cd));
        #endif
        default:
            return in;
        }
    }
    

} // impl_icu
} // locale
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
