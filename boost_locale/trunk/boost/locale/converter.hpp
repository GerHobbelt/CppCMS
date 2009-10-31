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
        ///
        /// This class is big ugly hook for DLL in order to make sure that both program and DLL
        /// refer to same locale::id when it uses some atomic static members.
        ///
        /// Further we specialize it for char, wchar_t, char16_t and char32_t in order to make them work.
        ///
        template<typename CharType>
        struct base_converter_facet : public std::locale::facet
        {
        };

        template<typename CharType>
        class converter : 
            public base_converter_facet<CharType>,
            public converter_base
        {
        public:
            
            typedef std::basic_string<CharType> string_type;
            typedef CharType char_type;

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

            string_type to_upper(char_type const *begin, char_type const *end) const
            {
                return convert(upper_case,begin,end);
            }
            
            string_type to_lower(char_type const *begin, char_type const *end) const
            {
                return convert(lower_case,begin,end);
            }

            string_type to_title(char_type const *begin, char_type const *end) const
            {
                return convert(title_case,begin,end);
            }

            string_type fold_case(char_type const *begin, char_type const *end) const
            {
                return convert(case_folding,begin,end);
            }

            string_type normalize(char_type const *begin, char_type const *end,normalization_type norm=norm_default) const
            {
                return convert(normalization,begin,end,norm);
            }
            
            
            string_type convert(conversion_type how,string_type const &str,int flags = 0) const
            {
                return do_convert(how,str.data(),str.data()+str.size(),flags);
            }
            
            string_type convert(conversion_type how,char_type const *begin,char_type const *end,int flags = 0) const
            {
                return do_convert(how,begin,end,flags);
            }

            
            static converter *create(info const &inf);
            
        protected:
            
            converter(size_t refs=0) : base_converter_facet<CharType>(refs)
            {
            }

            virtual ~converter()
            {
            }

            virtual string_type do_convert(conversion_type how,char_type const *begin,char_type const *end,int flags) const = 0;

        };

        template<>
        struct BOOST_LOCALE_DECL base_converter_facet<char> : public std::locale::facet 
        {
            base_converter_facet(size_t refs = 0) : std::locale::facet(refs)
            {
            }
            static std::locale::id id;
        };

        template<>
        BOOST_LOCALE_DECL converter<char> *converter<char>::create(info const &inf);
        
        #ifndef BOOST_NO_STD_WSTRING
        template<>
        struct BOOST_LOCALE_DECL base_converter_facet<wchar_t> : public std::locale::facet 
        {
            base_converter_facet(size_t refs = 0) : std::locale::facet(refs)
            {
            }
            static std::locale::id id;
        };

        template<>
        BOOST_LOCALE_DECL converter<wchar_t> *converter<wchar_t>::create(info const &inf);
        #endif
        
        #ifdef BOOST_HAS_CHAR16_T
        template<>
        struct BOOST_LOCALE_DECL base_converter_facet<char16_t> : public std::locale::facet 
        {
            base_converter_facet(size_t refs = 0) : std::locale::facet(refs)
            {
            }
            static std::locale::id id;
        };

        template<>
        BOOST_LOCALE_DECL converter<char16_t> *converter<char16_t>::create(info const &inf);
        #endif
        
        #ifdef BOOST_HAS_CHAR32_T
        template<>
        struct BOOST_LOCALE_DECL base_converter_facet<char32_t> : public std::locale::facet 
        {
            base_converter_facet(size_t refs = 0) : std::locale::facet(refs)
            {
            }
            static std::locale::id id;
        };

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
        std::basic_string<CharType> to_upper(CharType const *begin,std::locale const &loc=std::locale())
        {
            CharType const *end=begin;
            while(*end) end++;
            return std::use_facet<converter<CharType> >(loc).to_upper(begin,end);
        }
        
        template<typename CharType>
        std::basic_string<CharType> to_lower(std::basic_string<CharType> const &str,std::locale const &loc=std::locale())
        {
            return std::use_facet<converter<CharType> >(loc).to_lower(str);
        }
        
        template<typename CharType>
        std::basic_string<CharType> to_lower(CharType const *begin,std::locale const &loc=std::locale())
        {
            CharType const *end=begin;
            while(*end) end++;
            return std::use_facet<converter<CharType> >(loc).to_lower(begin,end);
        }
        
        template<typename CharType>
        std::basic_string<CharType> to_title(std::basic_string<CharType> const &str,std::locale const &loc=std::locale())
        {
            return std::use_facet<converter<CharType> >(loc).to_title(str);
        }
        
        template<typename CharType>
        std::basic_string<CharType> to_title(CharType const *begin,std::locale const &loc=std::locale())
        {
            CharType const *end=begin;
            while(*end) end++;
            return std::use_facet<converter<CharType> >(loc).to_title(begin,end);
        }
        
        template<typename CharType>
        std::basic_string<CharType> fold_case(std::basic_string<CharType> const &str,std::locale const &loc=std::locale())
        {
            return std::use_facet<converter<CharType> >(loc).fold_case(str);
        }
        
        template<typename CharType>
        std::basic_string<CharType> fold_case(CharType const *begin,std::locale const &loc=std::locale())
        {
            CharType const *end=begin;
            while(*end) end++;
            return std::use_facet<converter<CharType> >(loc).fold_case(begin,end);
        }
        
        template<typename CharType>
        std::basic_string<CharType> normalize(  std::basic_string<CharType> const &str,
                                                normalization::normalization_type nt=normalization::norm_default,
                                                std::locale const &loc=std::locale())
        {
            return std::use_facet<converter<CharType> >(loc).normalize(str,nt);
        }
        
        template<typename CharType>
        std::basic_string<CharType> normalize(  CharType const *begin,
                                                normalization::normalization_type nt=normalization::norm_default,
                                                std::locale const &loc=std::locale())
        {
            CharType const *end=begin;
            while(*end) end++;
            return std::use_facet<converter<CharType> >(loc).normalize(begin,end,nt);
        }

    }

}


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

