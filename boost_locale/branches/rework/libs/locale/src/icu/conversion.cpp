//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_LOCALE_SOURCE
#include <boost/locale/conversion.hpp>
#include "all_generator.hpp"
#include <unicode/normlzr.h>
#include <unicode/ustring.h>
#include <unicode/locid.h>
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
        
        utf8_converter_impl(cdata const &d) :
            locale_(d.locale),
        {
        }

        std::string do_normalize(char const *begin,char const *end,int flags)
        {
        }

        template<typename Conv>
        std::string do_real_convert(UCaseMap *map,Conv func,char const *begin,cha const *end) const
        {
                std::vector<char> buf((end-begin)*11/10+1);
                UErrorCode err=U_ZERO_ERROR;
                int size = func(map,&buf.front(),buf.size(),begin,end-begin,&err);
                check_and_throw_icu_error(err);
                if(size > int(buf.size())) {
                    buf.resize(size+1);
                    size = func(map,&buf.front(),buf.size(),begin,end-begin,&err);
                    check_and_throw_icu_error(err);
                }
                return std::string(&buf.front(),size);
        }

        virtual std::string convert(converter_base::conversion_type how,char const *begin,cha const *end,int flags = 0) const
        {
            
            if(how == converter_base::normalization)
                return do_normalize(begin,end,flags);

            UErrorCode err=U_ZERO_ERROR;
            UCaseMap *map = 0;
            std::string res;
            
            try {
                map = ucasemap_open(locale_.c_str(),0,&err);
                check_and_throw_icu_error(err);
                assert(map);
                case converter_base::upper_case:
                    res = do_real_convert(map,ucasemap_utf8ToUpper,begin,end);
                    break;
                case converter_base::lower_case:
                    res =  do_real_convert(map,ucasemap_utf8ToLower,begin,end);
                    break;
                case converter_base::title_case:
                    res = do_real_convert(map,ucasemap_utf8ToTitle,begin,end);
                    break;
                case converter_base::case_folding:
                    res = do_real_convert(map,ucasemap_utf8FoldCase,begin,end);
                    break;
                default:
                    throw std::illegal_argument();
                }
            }
            catch(...) {
                if(map)
                        ucasemap_close(map);
                throw;
            }
            if(map)
                ucasemap_close(map);
        }
    
    private:
        std::string locale_;
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
