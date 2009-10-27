#define BOOST_LOCALE_SOURCE
#include <boost/locale/formatting.hpp>
#include <boost/locale/formatter.hpp>
#include <typeinfo>
#include "formatting_info.hpp"
#include "ios_prop.hpp"

namespace boost {
    namespace locale {

        namespace {
            impl::ios_info &info(std::ios_base &ios)
            {
                return impl::ios_prop<impl::ios_info>::get(ios);
            }

            template<typename Char>
            void do_ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id, std::basic_string<Char> const &pattern)
            {
                switch(pattern_id) {
                case flags::datetime_pattern:
                    info(ios).datetime(pattern);
                    break;
                default:
                    throw std::bad_cast();
                }
            }
            
            template<typename Char>
            std::basic_string<Char> do_ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id)
            {
                switch(pattern_id) {
                case flags::datetime_pattern:
                    return info(ios).datetime<Char>();
                default:
                    throw std::bad_cast();
                }
            }
        }

        BOOST_LOCALE_DECL uint64_t ext_flags(std::ios_base &ios)
        {
            return info(ios).flags();
        }
        BOOST_LOCALE_DECL uint64_t ext_flags(std::ios_base &ios,flags::display_flags_type mask)
        {
            return info(ios).flags() & mask;
        }
        
        BOOST_LOCALE_DECL void ext_setf(std::ios_base &ios,flags::display_flags_type flags,flags::display_flags_type mask)
        {
            impl::ios_info &inf=info(ios);
            inf.flags((inf.flags() & ~(uint64_t(mask))) | flags);
        }
        
        BOOST_LOCALE_DECL int ext_value(std::ios_base &ios,flags::value_type id)
        {
            impl::ios_info &inf=info(ios);
            return inf.value(id);
        }
        BOOST_LOCALE_DECL void ext_value(std::ios_base &ios,flags::value_type id,int value)
        {
            impl::ios_info &inf=info(ios);
            inf.value(id,value);
        }


        template<>
        BOOST_LOCALE_DECL void ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id, std::string const &pattern)
        {
            if(pattern_id == flags::time_zone_id)
                info(ios).timezone(pattern);
            else
                do_ext_pattern(ios,pattern_id,pattern);
        }
        
        template<>
        BOOST_LOCALE_DECL std::string ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id)
        {
            if(pattern_id == flags::time_zone_id)
                return info(ios).timezone();
            else
                return do_ext_pattern<char>(ios,pattern_id);
        }

        #ifndef BOOST_NO_STD_WSTRING
        
        template<>
        BOOST_LOCALE_DECL void ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id, std::wstring const &pattern)
        {
            do_ext_pattern(ios,pattern_id,pattern);
        }

        template<>
        BOOST_LOCALE_DECL std::wstring ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id)
        {
            return do_ext_pattern<wchar_t>(ios,pattern_id);
        }

        #endif // BOOST_NO_STD_WSTRING

        #ifdef BOOST_HAS_CHAR16_T
        template<>
        BOOST_LOCALE_DECL void ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id, std::u16string const &pattern)
        {
            do_ext_pattern(ios,pattern_id,pattern);
        }

        template<>
        BOOST_LOCALE_DECL std::u16string ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id)
        {
            return do_ext_pattern<char16_t>(ios,pattern_id);
        }
        #endif // char16_t, u16string

        #ifdef BOOST_HAS_CHAR32_T
        template<>
        BOOST_LOCALE_DECL void ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id, std::u32string const &pattern)
        {
            do_ext_pattern(ios,pattern_id,pattern);
        }

        template<>
        BOOST_LOCALE_DECL std::u32string ext_pattern(std::ios_base &ios,flags::pattern_type pattern_id)
        {
            return do_ext_pattern<char32_t>(ios,pattern_id);
        }
        #endif // char32_t, u32string
    } // locale
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
