#ifndef BOOST_LOCALE_CODEPAGE_HPP_INCLUDED
#define BOOST_LOCALE_CODEPAGE_HPP_INCLUDED

namespace boost {
    namespace locale {
        namespace details {
            struct unicode_buffer;
            template<typename CharType>
            struct to_converter {
                unicode_buffer &operator()(CharType const *begin,CharType const *end);
            };
            template<typename CharType>
            struct from_converter {
                std::basic_string<CharType> operator()(unicode_buffer &buffer);
            };
        };
        template<typename CharIn,typename CharOut>
        class codepage {
        public:
            typedef std::basic_string<CharIn> in_string_type;
            typedef std::basic_string<CharOut> out_string_type;
            out_string_type operator()(in_string_type const &str)
            {
            }
            out_string_type operator()(CharIn const *begin,CharIn const *end);
            codepage_
        };
    }
}


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

