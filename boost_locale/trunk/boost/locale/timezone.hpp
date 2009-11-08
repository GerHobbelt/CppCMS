#ifndef BOOST_LOCALE_TIMEZONE_HPP_INCLUDED
#define BOOST_LOCALE_TIMEZONE_HPP_INCLUDED

#include <boost/locale/config.hpp>
#include <string>
#include <set>
#include <ostream>
#include <memory>

namespace boost {
    namespace locale {

        class time_zone_impl;

        ///
        /// \brief class used for calculation of time zone offsets
        ///
        class time_zone {
        public:
            
            ///
            /// Create GMT Time Zone
            ///
            time_zone();

            time_zone(time_zone const &other);
            time_zone const &operator=(time_zone const &other);
            ~time_zone();
            ///
            /// Create time zone with id \a id
            ///
            time_zone(std::string const &id);

            bool operator==(time_zone const &other) const;

            bool operator!=(time_zone const &other) const
            {
                return !(*this==other);
            }


            ///
            /// Get time zone id
            /// 
            std::string id() const;

            ///
            /// Find an offset in seconds from GMT for time \a time, If \a time represents locale time \a is_locale_time
            /// should be set to true
            /// 
            double offset_from_gmt(double time,bool is_local_time = false) const;

            ///
            /// Adjust locale time to GMT time
            ///
            template<typename TimeType>
            TimeType to_gmt(TimeType local_time) const
            {
                return local_time - offset_from_gmt(static_cast<double>(local_time),true);
            }

            ///
            /// Adjust GMT time to local time
            ///
            template<typename TimeType>
            TimeType to_local(TimeType gmt_time) const
            {
                return gmt_time + offset_from_gmt(static_cast<double>(gmt_time),false);
            }
            
            ///
            /// Get a list of all supported time zone ids
            ///
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

        ///
        /// Write time zone in human readable format to stream. Note this is not the same as switching time zone of the
        /// stream. If you want to switch time zone use manipulator as::time_zone
        ///
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
