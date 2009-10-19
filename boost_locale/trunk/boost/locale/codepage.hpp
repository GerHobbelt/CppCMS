#ifndef BOOST_LOCALE_CODEPAGE_HPP_INCLUDED
#define BOOST_LOCALE_CODEPAGE_HPP_INCLUDED

namespace boost {
    namespace locale {
        class codepage : public std::locale::facet {
        public:
            static std::locale::id id;

            template<typename CharOut,typename CharIn>
            std::basic_string<CharOut> encode(CharIn const *begin,CharIn const *end)
            {
                
            }
            
            template<typename CharOut,typename CharIn>
            virtual std::basic_string<CharOut> encode(std::basic_string<CharIn> const &str)
            {
                return encode<CharOut>(str.data(),str.data()+str.size());
            }

        };
    }
}


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

