#ifndef BOOST_LOCALE_TIMEZONE_HPP_INCLUDED
#define BOOST_LOCALE_TIMEZONE_HPP_INCLUDED

#include <boost/locale/config.hpp>
#include <string>
#include <set>
#include <ostream>

namespace boost {
    namespace locale {

        class time_zone_impl;
        class time_zone {
        public:
            
            ///
            /// Create GMT Time Zo
            ///
            time_zone();
            time_zone(time_zone const &other);
            time_zone const &operator=(time_zone const &other);
            ~time_zone();
            time_zone(std::string const &id);

            bool operator==(time_zone const &other) const;

            bool operator!=(time_zone const &other) const
            {
                return !(*this==other);
            }


            std::string id() const;

            double offset_from_gmt(double time,bool is_local_time = false) const;

            template<typename TimeType>
            TimeType to_gmt(TimeType local_time) const
            {
                return local_time - offset_from_gmt(static_cast<double>(local_time),true);
            }

            template<typename TimeType>
            TimeType to_local(TimeType gmt_time) const
            {
                return gmt_time + offset_from_gmt(static_cast<double>(gmt_time),false);
            }
            
            static std::set<std::string> all_zones();

            ///
            /// For internal Use only -- don't use it
            ///

            time_zone_impl *impl() const
            {
                return impl_.get();
            }
        private:
            std::auto_ptr<time_zone_impl> impl_;
        };

        template<typename CharType>
        std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out,time_zone const &tz);
        
        template<>
        BOOST_LOCALE_DECL std::basic_ostream<char> &operator<<(std::basic_ostream<char> &out,time_zone const &tz);
    
        #ifndef BOOST_NO_STD_WSTRING
        template<>
        BOOST_LOCALE_DECL std::basic_ostream<wchar_t> &operator<<(std::basic_ostream<wchar_t> &out,time_zone const &tz);
        #endif
    
        
        #ifdef BOOST_HAS_CHAR16_T
        template<>
        BOOST_LOCALE_DECL std::basic_ostream<u16char_t> &operator<<(std::basic_ostream<u16char_t> &out,time_zone const &tz);
        #endif
    
        #ifdef BOOST_HAS_CHAR32_T
        template<>
        BOOST_LOCALE_DECL std::basic_ostream<u32char_t> &operator<<(std::basic_ostream<u32char_t> &out,time_zone const &tz);
        #endif
    
    }
}

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
