#define BOOST_LOCALE_SOURCE
#include <boost/locale/converter.hpp>
#include <boost/locale/info.hpp>
#include <boost/noncopyable.hpp>
#include <unicode/normlzr.h>
#include <unicode/locid.h>

#include "info_impl.hpp"
#include "uconv.hpp"


namespace boost {
    namespace locale {
        class converter_impl : public boost::noncopyable {
        public:

            converter_impl(icu::Locale const &locale) : 
                locale_(locale)
            {
            }

            void convert(icu::UnicodeString &str,converter_base::conversion_type how,int flags) const
            {
                switch(how) {
                case converter_base::upper_case:
                    str.toUpper(locale_);
                    return;
                case converter_base::lower_case:
                    str.toLower(locale_);
                    return;
                case converter_base::title_case:
                    str.toTitle(0,locale_);
                    return;
                case converter_base::case_folding:
                    str.foldCase();
                    return;
                case converter_base::normalization:
                    normalize(str,flags);
                    return;
                }
            }

            void normalize(icu::UnicodeString &str,int flags) const
            {
                UErrorCode code=U_ZERO_ERROR;
                UNormalizationMode mode=UNORM_DEFAULT;
                switch(flags) {
                case converter_base::norm_nfd:
                    mode=UNORM_NFD;
                    break;
                case converter_base::norm_nfc:
                    mode=UNORM_NFC;
                    break;
                case converter_base::norm_nfkd:
                    mode=UNORM_NFKD;
                    break;
                case converter_base::norm_nfkc:
                    mode=UNORM_NFKC;
                    break;
                }
                icu::UnicodeString tmp;
                icu::Normalizer::normalize(str,mode,0,tmp,code);

                impl::check_and_throw_icu_error(code);

                str=tmp;
            }
        private:
            icu::Locale locale_;
        };


        template<typename CharType>
        class converter_derived : public converter<CharType>
        {
        public:
            converter_derived(icu::Locale const &locale,std::string charset,size_t refs = 0) :
                converter<CharType>(refs),
                impl_(locale),
                code_page_converter_(charset)
            {
            }
            typedef typename converter<CharType>::string_type string_type;
            typedef typename converter<CharType>::char_type char_type;
            typedef typename converter<CharType>::conversion_type conversion_type;	        

            virtual string_type do_convert(conversion_type how,char_type const *begin,char_type const *end,int flags) const
            {
                icu::UnicodeString temp(code_page_converter_.icu(begin,end));
                impl_.convert(temp,how,flags);
                return code_page_converter_.std(temp);
            } 

        private:
            converter_impl impl_;
            impl::icu_std_converter<CharType> code_page_converter_;
        };

        template<typename CharType>
        converter<CharType> *create_converter(info const &inf)
        {
            return new converter_derived<CharType>(inf.impl()->locale,inf.encoding());
        }
        
        template<>
        BOOST_LOCALE_DECL converter<char> *converter<char>::create(info const &inf)
        {
            return create_converter<char>(inf);
        }

        #ifndef BOOST_NO_STD_WSTRING
        template<>
        BOOST_LOCALE_DECL converter<wchar_t> *converter<wchar_t>::create(info const &inf)
        {
            return create_converter<wchar_t>(inf);
        }
        #endif
        
        #ifdef BOOST_HAS_CHAR16_T
        template<>
        BOOST_LOCALE_DECL converter<char16_t> *converter<char16_t>::create(info const &inf)
        {
            return create_converter<char16_t>(inf);
        }
        #endif
        
        #ifdef BOOST_HAS_CHAR32_T
        template<>
        BOOST_LOCALE_DECL converter<char32_t> *converter<char32_t>::create(info const &inf)
        {
            return create_converter<char32_t>(inf);
        }
        #endif


    } // locale
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
