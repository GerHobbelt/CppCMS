#include <boost/locale/converter.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <unicode/normlzr.h>

#include "uconv.hpp"


namespace boost {
    namespace locale {
        class converter_impl : public boost::noncopyable {
        public:

            typedef boost::shared_ptr<icu::Locale> locale_pointer_type;

            converter_impl(locale_pointer_type locale) : 
                locale_(locale)
            {
            }

            void convert(icu::UnicodeString &str,converter_base::conversion_type how,int flags) const
            {
                switch(how) {
                case converter_base::upper_case:
                    str.toUpper(*locale_);
                    return;
                case converter_base::lower_case:
                    str.toLower(*locale_);
                    return;
                case converter_base::title_case:
                    str.toTitle(0,*locale_);
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
            locale_pointer_type locale_;
        };


        template<typename CharType>
        class converter_derived : public converter<CharType>
        {
        public:
            converter_derived(boost::shared_ptr<icu::Locale> locale,std::string charset,size_t refs = 0) :
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
        converter<CharType> *generate(boost::shared_ptr<icu::Locale> locale,std::string charset)
        {
            return new converter_derived<CharType>(locale,charset);
        }

        std::locale generate_all()
        {
            boost::shared_ptr<icu::Locale> locale;
            std::string set;
            std::locale a("");
            std::locale b(a,generate<char>(locale,set));
            std::locale c(b,generate<wchar_t>(locale,set));
            std::locale d(c,generate<char16_t>(locale,set));
            std::locale e(d,generate<char32_t>(locale,set));
	    return e;
	 }


    } // locale
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
