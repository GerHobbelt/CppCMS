#ifndef BOOST_LOCALE_FORMATTING_HPP_INCLUDED
#define BOOST_LOCALE_FORMATTING_HPP_INCLUDED

#include <boost/config.hpp>

namespace boost {
    namespace locale {
        namespace flags {
            typedef enum {
                posix               = 0,
                number              = 1,
                currency            = 2,
                percent             = 3,
                date                = 4
                time                = 5,
                datetime            = 6,
                strftime            = 7,
                spellout            = 8,
                ordinal             = 9,
                duration            = 10,

                display_flags_mask  = 31,

                currency_default    = 0 << 5,
                currency_iso        = 1 << 5,
                currency_national   = 2 << 5,

                currency_flags_mask = 3 << 5

                time_default        = 0 << 7,
                time_short          = 1 << 7,
                time_medium         = 2 << 7,
                time_long           = 3 << 7,
                time_full           = 4 << 7,
                time_flags_mask     = 7 << 7,

                date_default        = 0 << 10,
                date_short          = 1 << 10,
                date_medium         = 2 << 10,
                date_long           = 3 << 10,
                date_full           = 4 << 10,
                date_flags_mask     = 7 << 10,

                datetime_flags_mask = date_flags_mask | time_flags_mask;

            } display_flags_type;

            typedef enum {
                datetime_pattern,
                separator_pattern
            } pattern_type;

            
        } // flags

        uint64_t ext_flags(std::ios_base &);
        uint64_t ext_flags(std::ios_base &,flags::display_flags_type mask);
        void ext_setf(std::ios_base &,flags::display_flags_type flags,flags::display_flags_type mask);


       
        template<typename CharType>
        void ext_pattern(std::ios_base &,flags::pattern_type pat,std::basic_string<CharType> const &);

        template<typename CharType>
        std::basic_string<CharType> ext_pattern(std::ios_base &,flags::pattern_type pattern);

        /// Specializations

        template<>
        void ext_pattern(std::ios_base &,flags::pattern_type pattern_id, std::string const &pattern);
        
        template<>
        std::string ext_pattern(std::ios_base &,flags::pattern_type pattern_id);

        #ifndef BOOST_NO_STD_WSTRING
        
        template<>
        void ext_pattern(std::ios_base &,flags::pattern_type pattern_id, std::wstring const &pattern);

        template<>
        std::wstring ext_pattern(std::ios_base &,flags::pattern_type pattern_id);

        #endif // BOOST_NO_STD_WSTRING

        #ifdef BOOST_HAS_CHAR16_T
        template<>
        void ext_pattern(std::ios_base &,flags::pattern_type pattern_id, std::u16string const &pattern);

        template<>
        std::u16string ext_pattern(std::ios_base &,flags::pattern_type pattern_id);
        #endif // char16_t, u16string

        #ifdef BOOST_HAS_CHAR32_T
        template<>
        void ext_pattern(std::ios_base &,flags::pattern_type pattern_id, std::u32string const &pattern);

        template<>
        std::u32string ext_pattern(std::ios_base &,flags::pattern_type pattern_id);
        #endif // char32_t, u32string


        namespace as {

            #define BOOST_LOCALE_AS_MANIPULATOR(name,mask)  \
            inline std::ios_base &name(std::ios_base &ios)  \
            {                                               \
                ext_setf(ios,flags::name,flags::mask)       \
                return ios;                                 \
            }

            BOOST_LOCALE_AS_MANIPULATOR(posix,display_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(number,display_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(currency,display_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(percent,display_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(date,display_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(time,display_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(datetime,display_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(strftime,display_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(spellout,display_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(ordinal,display_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(duration,display_flags_mask)

            BOOST_LOCALE_AS_MANIPULATOR(currency_default,currency_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(currency_iso,currency_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(currency_national,currency_flags_mask)
            
            BOOST_LOCALE_AS_MANIPULATOR(time_default,time_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(time_short,time_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(time_medium,time_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(time_long,time_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(time_full,time_flags_mask)
            
            BOOST_LOCALE_AS_MANIPULATOR(date_default,date_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(date_short,date_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(date_medium,date_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(date_long,date_flags_mask)
            BOOST_LOCALE_AS_MANIPULATOR(date_full,date_flags_mask)

            
            namespace details {
                template<typename CharType>
                struct add_ftime {

                    std::basic_string<CharType> ftime;

                    template<typename CharType>
                    void apply(std::base_ios<CharType> &ios) const
                    {
                        ext_pattern(ios,flags::datetime_pattern,ftime);
                        as::strftime(ios);
                    }

                }
                template<typename CharType>
                std::basic_ostream<CharType> &operator<<(std::basic_ostream &out,add_ftime<CharType> const &fmt)
                {
                    fmt.apply(out);
                    return out;
                }
                
                template<typename CharType>
                std::basic_istream<CharType> &operator>>(std::basic_istream &in,add_ftime<CharType> const &fmt)
                {
                    fmt.apply(in);
                    return in;
                }

            }

            template<typename CharType>
            details::add_ftime<CharType> ftime(std::basic_string<CharType> const &format)
            {
                details::add_ftime<CharType> fmt;
                fmt.ftime=format;
                return fmt;
            }

            namespace details {
                template<typename CharType>
                struct add_separator {

                    std::basic_string<CharType> sep;

                    void apply(std::ios &ios) const
                    {
                        std::basic_ostream<CharType> pat=ext_pattern(ios,flags::separator_pattern);
                        if(!pat.empty())
                            pat.append(ios.widen('\n'));
                        pat+=sep;
                        ext_pattern(ios,flags::separator_pattern,pat)
                    }

                }
                template<typename CharType>
                std::basic_ostream<CharType> &operator<<(std::basic_ostream &out,add_separator<CharType> const &fmt)
                {
                    fmt.apply(out);
                    return out;
                }
                
                template<typename CharType>
                std::basic_istream<CharType> &operator>>(std::basic_istream &in,add_separator<CharType> const &fmt)
                {
                    fmt.apply(in);
                    return in;
                }

            } // details 

            template<typename CharType>
            details::add_separator<CharType> separate(std::basic_string<CharType> const &sep)
            {
                return details::add_separator<CharType>(sep); 
            }

            template<typename CharType>
            std::base_ios<CharType> &noseparators(std::base_ios<CharType> &ios)
            {
                ext_pattern(ios,flags::separator_pattern,std::basic_string<CharType>());
                return ios;
            }



        } // as manipulators
        
        #undef BOOST_LOCALE_AS_MANIPULATOR



    } // locale
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
