#ifndef BOOST_LOCALE_BOUNDARY_HPP_INCLUDED
#define BOOST_LOCALE_BOUNDARY_HPP_INCLUDED

#include <boost/locale/config.hpp>
#include <boost/cstdint.hpp>

#include <locale>
#include <vector>

namespace boost {
    namespace locale {
        
        struct boundary {
            typedef enum {
                character,
                word,
                sentence,
                line
            } boundary_type;

            typedef enum {
                number  = 1 << 0,
                letter  = 1 << 1,
                kana    = 1 << 2,
                ideo    = 1 << 3,
            } word_type;
            typedef enum {
                soft    = 1 << 0,
                hard    = 1 << 1,
            } break_type;
            
            struct break_info {
                break_info() : 
                    offset(0),
                    next(0),
                    prev(0),
                    brk (0),
                    reserved(0)
                {
                }
                
                break_info(unsigned v) :
                    offset(v),
                    next(0),
                    prev(0),
                    brk (0),
                    reserved(0)
                {
                }

                uint32_t offset;
                uint32_t next       : 10;
                uint32_t prev       : 10;
                uint32_t brk        : 10;
                uint32_t reserved   : 2;
                
                bool operator<(break_info const &other) const
                {
                    return offset < other.offset;
                }
            };
            
            typedef std::vector<break_info> index_type;

            template<typename CharType>
            static index_type map(boundary_type t,CharType const *begin,CharType const *end,std::locale const &loc);
            
            template<typename CharType>
            static index_type map(
                            boundary_type t,
                            std::basic_string<CharType> const &str,
                            std::locale const &loc=std::locale())
            {
                return map(t,str.data(),str.data()+str.size(),loc);
            }
        };
        
        template<>
        BOOST_LOCALE_DECL boundary::index_type 
        boundary::map(boundary::boundary_type t,char const *begin,char const *end,std::locale const &loc);

        #ifndef BOOST_NO_STD_WSTRING
        template<>
        BOOST_LOCALE_DECL boundary::index_type 
        boundary::map(boundary::boundary_type t,wchar_t const *begin,wchar_t const *end,std::locale const &loc);
        #endif

        #ifdef BOOST_HAS_CHAR16_T
        template<>
        BOOST_LOCALE_DECL boundary::index_type 
        boundary::map(boundary::boundary_type t,char16_t const *begin,char16_t const *end,std::locale const &loc);
        #endif

        #ifdef BOOST_HAS_CHAR32_T
        template<>
        BOOST_LOCALE_DECL boundary::index_type 
        boundary::map(boundary::boundary_type t,char32_t const *begin,char32_t const *end,std::locale const &loc);
        #endif

    }
}

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
