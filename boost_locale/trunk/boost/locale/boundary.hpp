#ifndef BOOST_LOCALE_BOUNDARY_HPP_INCLUDED
#define BOOST_LOCALE_BOUNDARY_HPP_INCLUDED

#include <boost/locale/config.hpp>
#include <boost/cstdint.hpp>

#include <locale>
#include <vector>

namespace boost {
    namespace locale {
       
        ///
        /// \brief This structure hold all the types required for boundary analysis
        ///
        
        struct boundary {
            ///
            /// \brief The enum that describes possible break types
            ///
            typedef enum {
                character,  ///< Find character boundaries
                word,       ///< Find word boundaries
                sentence,   ///< Find sentence boundaries
                line        ///< Find a positions suitable for line breaks
            } boundary_type;

            ///
            /// Flags used with word boundary analysis -- the type of word found
            ///
            typedef enum {
                number  = 1 << 0,   ///< Word that appear to be a number
                letter  = 1 << 1,   ///< Word that contains letters
                kana    = 1 << 2,   ///< Word that contains kana characters
                ideo    = 1 << 3,   ///< Word that contains ideographic characters
            } word_type;
            ///
            /// Flags that describe a type of line break
            ///
            typedef enum {
                soft    = 1 << 0,   ///< Optional line break
                hard    = 1 << 1,   ///< Mandatory line break
            } break_type;
            
            ///
            /// \ brief a structure that describes break position
            /// 
            struct break_info {

                ///
                /// Default constructor -- all fields are zero
                /// 
                break_info() : 
                    offset(0),
                    next(0),
                    prev(0),
                    brk (0),
                    reserved(0)
                {
                }
                ///
                /// Create a break info with offset \a v
                ///
                break_info(unsigned v) :
                    offset(v),
                    next(0),
                    prev(0),
                    brk (0),
                    reserved(0)
                {
                }

                uint32_t offset;            ///< Offset from the beginning of the text where break occurs
                uint32_t next       : 10;   ///< The flag describing a word following the break position
                uint32_t prev       : 10;   ///< The flag describing a word preceding the break position
                uint32_t brk        : 10;   ///< The flag describing a type of line break
                uint32_t reserved   : 2;
               
                ///
                /// Comparison operator allowing usage of break_info in STL algorithms
                /// 
                bool operator<(break_info const &other) const
                {
                    return offset < other.offset;
                }
            };
           
            ///
            /// The index of all found boundaries. Note: for a string with length \a len, 0 and len are always considered
            /// boundaries
            /// 
            typedef std::vector<break_info> index_type;

            template<typename CharType>

            ///
            /// Find boundary positions of type \a t for text in range [begin,end) using locale \a loc
            ///
            static index_type map(boundary_type t,CharType const *begin,CharType const *end,std::locale const &loc=std::locale());
            
            ///
            /// Find boundary positions of type \a t for string \a str using locale \a loc
            ///
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
