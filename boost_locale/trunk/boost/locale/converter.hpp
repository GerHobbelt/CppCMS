#ifndef BOOST_LOCALE_CONVERTER_HPP_INCLUDED
#define BOOST_LOCALE_CONVERTER_HPP_INCLUDED

#include <boost/locale/config.hpp>
#include <locale>

namespace boost {
    namespace locale {

        struct normalization {
            typedef enum {
                norm_default = 0,
                norm_nfd,
                norm_nfc,
                norm_nfkd,
                norm_nfkc
            } normalization_type;
        };

        class converter_base : public normalization {
        public:

            typedef enum {
                normalization,
                upper_case,
                lower_case,
                case_folding,
                title_case
            } conversion_type;


        };

        class info;

        template<typename CharType>
        class converter : 
            public std::locale::facet,
            public converter_base
        {
        public:
            
            typedef std::basic_string<CharType> string_type;
            typedef CharType char_type;

            static std::locale::id id;

       
            string_type to_upper(string_type const &str) const
            {
                return convert(upper_case,str);
            }
            
            string_type to_lower(string_type const &str) const
            {
                return convert(lower_case,str);
            }

            string_type to_title(string_type const &str) const
            {
                return convert(title_case,str);
            }

            string_type fold_case(string_type const &str) const
            {
                return convert(case_folding,str);
            }

            string_type normalize(string_type const &str,normalization_type norm=norm_default) const
            {
                return convet(normalization,str,norm);
            }

            string_type to_upper(char const *begin, char const *end) const
            {
                return convert(upper_case,begin,end);
            }
            
            string_type to_lower(char const *begin, char const *end) const
            {
                return convert(lower_case,begin,end);
            }

            string_type to_title(char const *begin, char const *end) const
            {
                return convert(title_case,begin,end);
            }

            string_type fold_case(char const *begin, char const *end) const
            {
                return convert(case_folding,begin,end);
            }

            string_type normalize(char const *begin, char const *end,normalization_type norm=norm_default) const
            {
                return convert(normalization,begin,end,norm);
            }
            
            
            string_type convert(conversion_type how,std::string const &str,int flags = 0) const
            {
                return do_convert(how,str.data(),str.data()+str.size(),flags);
            }
            
            string_type convert(conversion_type how,char_type const *begin,char_type const *end,int flags = 0) const
            {
                return do_convert(how,begin,end,flags);
            }

            
            static converter *create(info const &inf);
            
        protected:
            
            converter(size_t refs=0) : std::locale::facet(refs)
            {
            }

            virtual ~converter()
            {
            }

            virtual string_type do_convert(conversion_type how,char_type const *begin,char_type const *end,int flags) const = 0;

        };

        template<>
        BOOST_LOCALE_DECL converter<char> *converter<char>::create(info const &inf);
        #ifndef BOOST_NO_STD_WSTRING
        template<>
        BOOST_LOCALE_DECL converter<wchar_t> *converter<wchar_t>::create(info const &inf);
        #endif
        
        #ifdef BOOST_HAS_CHAR16_T
        template<>
        BOOST_LOCALE_DECL converter<char16_t> *converter<char16_t>::create(info const &inf);
        #endif
        
        #ifdef BOOST_HAS_CHAR32_T
        template<>
        BOOST_LOCALE_DECL converter<char32_t> *converter<char32_t>::create(info const &inf);
        #endif

        ///
        /// Free function to_upper
        ///

        template<typename CharType>
        std::basic_string<CharType> to_upper(std::basic_string<CharType> const &str,std::locale const &loc=std::locale())
        {
            return std::use_facet<converter<CharType> >(loc).to_upper(str);
        }
        
        template<typename CharType>
        std::basic_string<CharType> to_lower(std::basic_string<CharType> const &str,std::locale const &loc=std::locale())
        {
            return std::use_facet<converter<CharType> >(loc).to_lower(str);
        }
        
        template<typename CharType>
        std::basic_string<CharType> to_title(std::basic_string<CharType> const &str,std::locale const &loc=std::locale())
        {
            return std::use_facet<converter<CharType> >(loc).to_title(str);
        }
        
        template<typename CharType>
        std::basic_string<CharType> fold_case(std::basic_string<CharType> const &str,std::locale const &loc=std::locale())
        {
            return std::use_facet<converter<CharType> >(loc).fold_case(str);
        }
        
        template<typename CharType>
        std::basic_string<CharType> normalize(  std::basic_string<CharType> const &str,
                                                normalization::normalization_type nt=normalization::norm_default,
                                                std::locale const &loc=std::locale())
        {
            return std::use_facet<converter<CharType> >(loc).normalize(str,nt);
        }

    }

}


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

