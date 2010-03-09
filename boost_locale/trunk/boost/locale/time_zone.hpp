//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
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
        /// \addtogroup date_time
        ///
        /// @{

        ///
        /// \brief class used for calculation of time zone offsets
        ///
        class time_zone {
        public:

            ///
            /// \brief Get a list of all supported time zone ids
            ///
            static std::set<std::string> all_zones();

            ///
            /// \brief Set default time zone
            ///
            /// Please note, this function does not change system time zone but all time zones related to this library/
            /// This function is not safe to use in multithreaded environment.
            ///
            static void global(time_zone const &zone);
 
            ///
            /// \brief Creates default Time Zone
            ///
            time_zone();

            ///
            /// \brief Copies time zone
            /// 
            time_zone(time_zone const &other);
            ///
            /// \brief Assigns time zone
            ///
            time_zone const &operator=(time_zone const &other);

            ~time_zone();
            ///
            /// Create time zone with id \a id
            ///
            time_zone(std::string const &id);

            ///
            /// Compares two time zones by ids
            ///
            bool operator==(time_zone const &other) const;

            ///
            /// Opposite of ==
            ///
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
            

            /// \cond INTERNAL

            //
            // For internal Use only -- don't use it
            //

            time_zone_impl *impl() const
            {
                return impl_.get();
            }

            /// \endcond
        private:
            std::auto_ptr<time_zone_impl> impl_;
        };

        ///
        /// Write time zone in human readable format to stream. Note this is not the same as switching time zone of the
        /// stream. If you want to switch time zone use manipulator as::time_zone
        ///
        template<typename CharType>
        std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out,time_zone const &tz);
        

        /// \cond INTERNAL
        template<>
        BOOST_LOCALE_DECL std::basic_ostream<char> &operator<<(std::basic_ostream<char> &out,time_zone const &tz);
    
        #ifndef BOOST_NO_STD_WSTRING
        template<>
        BOOST_LOCALE_DECL std::basic_ostream<wchar_t> &operator<<(std::basic_ostream<wchar_t> &out,time_zone const &tz);
        #endif
        
        #ifdef BOOST_HAS_CHAR16_T
        template<>
        BOOST_LOCALE_DECL std::basic_ostream<char16_t> &operator<<(std::basic_ostream<char16_t> &out,time_zone const &tz);
        #endif
    
        #ifdef BOOST_HAS_CHAR32_T
        template<>
        BOOST_LOCALE_DECL std::basic_ostream<char32_t> &operator<<(std::basic_ostream<char32_t> &out,time_zone const &tz);
        #endif
        /// \endcond
    
        ///
        /// @}
        ///

    } // locale
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
