//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_DATE_TIME_HPP_INCLUDED
#define BOOST_LOCALE_DATE_TIME_HPP_INCLUDED

#include <boost/locale/time_zone.hpp>
#include <locale>
#include <vector>
#include <stdexcept>


namespace boost {
    namespace locale {
        ///
        /// \defgroup date_time Date, Time, Timezone and Calendar manipulations 
        ///
        /// This module provides various calendar, timezone and date time services
        ///
        /// @{


        ///
        /// \brief This error is thrown in case of invalid state that occurred
        ///
        class date_time_error : public std::runtime_error {
        public:
            date_time_error(std::string const &e) : std::runtime_error(e) {}
        };

        namespace period {
            ///
            /// \brief This enum provides the list of various time periods that can be used for manipulation over date and time
            /// Operators like +, - * defined for these period allowing to perform easy calculations over them
            ///
            typedef enum {
                invalid,                    ///< Special invalid value, should not be used directs
                era,                        ///< Era i.e. AC, BC in Gregorian and Julian calendar, range [0,1]
                year,                       ///< Year, it is calendar specific
                extended_year,              ///< Extended year for Gregorian/Julian calendars, where 1 BC == 0, 2 BC == -1.
                month,                      ///< The month of year, calendar specific, in Gregorian [0..11]
                day,                        ///< The day of month, calendar specific, in Gregorian [1..31]
                day_of_year,                ///< The number of day in year, starting from 1
                day_of_week,                ///< Day of week, starting from Sunday, [1..7]
                day_of_week_in_month,       ///< Original number of the day of the week in month.
                day_of_week_local,          ///< Local day of week, for example in France Monday is 1, in US Sunday is 1, [1..7]
                hour,                       ///< 24 clock hour [0..23]
                hour_12,                    ///< 12 clock hour [0..11]
                am_pm,                      ///< am or pm marker, [0..1]
                minute,                     ///< minute [0..59]
                second,                     ///< second [0..59]
                week_of_year,               ///< The week number in the year
                week_of_month,              ///< The week number withing current month
            } period_type;
        } // period

        ///
        /// \brief This structure provides a pair period_type and amount. Usually obtained as product of period_type and integer.
        /// For example day*3 == date_time_period(day,3)
        /// 
        struct date_time_period 
        {
            period::period_type type;
            int value;
            date_time_period operator+() const { return *this; }
            date_time_period operator-() const { return date_time_period(type,-value); }
            
            date_time_period(period::period_type f=period::invalid,int v=1) : type(f), value(v) {}
        };

        inline date_time_period operator+(period::period_type f) 
        {
            return date_time_period(f);
        }
        inline date_time_period operator-(period::period_type f)
        {
            return date_time_period(f,-1);
        }

        /// \cond INTERNAL 
        #define BOOST_LOCALE_DATE_TIME_OPERATORS(Type) \
            inline date_time_period operator*(period::period_type f,Type v) { return date_time_period(f,v); } \
            inline date_time_period operator*(Type v,period::period_type f) { return date_time_period(f,v); } \
            inline date_time_period operator*(Type v,date_time_period f) { return date_time_period(f.type,f.value*v); } \
            inline date_time_period operator*(date_time_period f,Type v) { return date_time_period(f.type,f.value*v); }

        BOOST_LOCALE_DATE_TIME_OPERATORS(char)
        BOOST_LOCALE_DATE_TIME_OPERATORS(short int)
        BOOST_LOCALE_DATE_TIME_OPERATORS(int)
        BOOST_LOCALE_DATE_TIME_OPERATORS(long int)
        BOOST_LOCALE_DATE_TIME_OPERATORS(unsigned char)
        BOOST_LOCALE_DATE_TIME_OPERATORS(unsigned short int)
        BOOST_LOCALE_DATE_TIME_OPERATORS(unsigned int)
        BOOST_LOCALE_DATE_TIME_OPERATORS(unsigned long int)

        #undef BOOST_LOCALE_DATE_TIME_OPERATORS

        /// \endcond 

        ///
        /// \brief the class that represents a set of periods, It is generally created by operations on periods:
        /// 1995*year + 3*month + 1*day. Note: operations are not commutative.
        ///
        class date_time_period_set {
        public:
            date_time_period_set()
            {
            }
            date_time_period_set(period::period_type f)
            {
                basic_=date_time_period(f);
            }
            date_time_period_set(date_time_period const &fl)
            {
                basic_=fl;
            }
            void add(date_time_period f)
            {
                if(basic_.type==period::invalid)
                    basic_=f;
                else
                    periods_.push_back(f);
            }
            size_t size() const
            {
                return basic_.type==period::invalid ? 0 : 1 + periods_.size();
            }
            date_time_period const &operator[](int n) const 
            {
                if(n==0)
                    return basic_;
                else
                    return periods_[n-1]; 
            }
        private:
            date_time_period basic_;
            std::vector<date_time_period> periods_;
        };

        inline date_time_period_set operator+(date_time_period_set const &a,date_time_period_set const &b)
        {
            date_time_period_set s(a);
            for(unsigned i=0;i<b.size();i++)
                s.add(b[i]);
            return s;
        }
        
        inline date_time_period_set operator-(date_time_period_set const &a,date_time_period_set const &b)
        {
            date_time_period_set s(a);
            for(unsigned i=0;i<b.size();i++)
                s.add(-b[i]);
            return s;
        }


        ///
        /// \brief this class provides an access to general calendar information. That is not related to specific date
        ///
        class BOOST_LOCALE_DECL calendar {
        public:

            ///
            /// \brief Create calendar taking locale and timezone information from ios_base instance.
            /// 
            calendar(std::ios_base &ios);
            ///
            /// \brief Create calendar with locale \a l and time_zone \a zone
            ///
            calendar(std::locale const &l,time_zone const &zone);
            ///
            /// \brief Create calendar with locale \a l and default timezone
            ///
            calendar(std::locale const &l);
            ///
            /// \brief Create calendar with default locale and timezone \a zone
            ///
            calendar(time_zone const &zone);
            ///
            /// \brief Create calendar with default locale and timezone 
            ///
            calendar();
            ~calendar();

            ///
            /// \brief copy calendar
            ///
            calendar(calendar const &other);
            ///
            /// \brief assign calendar
            ///
            calendar const &operator=(calendar const &other);

            ///
            /// \brief Get minimum value for period f, For example for period::day it is 1.
            ///
            int minimum(period::period_type f) const;
            ///
            /// \brief Get grates possible minimum value for period f, For example for period::day it is 1, but may be different for other calendars.
            ///
            int greatest_minimum(period::period_type f) const;
            ///
            /// \brief Get maximum value for period f, For example for Gregorian calendar's maximum period::day it is 31.
            ///
            int maximum(period::period_type f) const;
            ///
            /// \brief Get least maximum value for period f, For example for Gregorian calendar's maximum period::day it is 28.
            ///
            int least_maximum(period::period_type f) const;

            ///
            /// \brief Get first day of week for specific calendar, for example for US it is 1 - Sunday for France it is 2 - Monday
            int first_day_of_week() const;

            ///
            /// \brief get calendar's locale
            ///
            std::locale get_locale() const;
            ///
            /// \brief get calendar's time zone
            ///
            time_zone get_time_zone() const;

            ///
            /// \brief Check if the calendar is Gregorian
            ///
            bool is_gregorian() const;

            ///
            /// \brief Compare calendars for equivalence: i.e. calendar types, time zones etc.
            ///
            bool operator==(calendar const &other) const;
            ///
            /// \brief Opposite of ==
            ///
            bool operator!=(calendar const &other) const;

        private:
            friend class date_time;
            std::locale locale_;
            boost::locale::time_zone tz_;
            void *impl_;
        };

        ///
        /// \brief this class represents a date time and allows to perform various operation according to the
        /// locale settings.
        ///
        
        class BOOST_LOCALE_DECL date_time {
        public:

            ///
            /// \brief default constructor, uses default calendar
            ///
            date_time();
            ///
            /// \brief copy date_time
            ///
            date_time(date_time const &other);
            ///
            /// \brief copy date_time and change some fields according to the \a set
            ///
            date_time(date_time const &other,date_time_period_set const &set);
            ///
            /// \brief assign the date_time
            ///
            date_time const &operator=(date_time const &other);
            ~date_time();

            ///
            /// \brief Create a date_time opject using POSIX time \a time and default calendar
            ///
            date_time(double time);
            ///
            /// \brief Create a date_time opject using POSIX time \a time and calendar \a cal
            ///
            date_time(double time,calendar const &cal);
            ///
            /// \brief Create a date_time opject using calendar \a cal
            ///
            date_time(calendar const &cal);
            
            ///
            /// \brief Create a date_time opject using default calendar and define values given in \a set
            ///
            date_time(date_time_period_set const &set);
            ///
            /// \brief Create a date_time opject using calendar \a cal and define values given in \a set
            ///
            date_time(date_time_period_set const &set,calendar const &cal);

           
            ///
            /// \brief assign values to valrious periods in set \a f  
            ///
            date_time const &operator=(date_time_period_set const &f);

            ///
            /// \brief set specific period \a f value to \a v
            ///
            void set(period::period_type f,int v);
            ///
            /// \brief get specific period \a f value
            ///
            int get(period::period_type f) const;

            ///
            /// \brief syntactic sugar for get(f)
            ///
            int operator/(period::period_type f) const
            {
                return get(f);
            }

            ///
            /// \brief add period f to the current date_time and return it
            ///
            date_time operator+(period::period_type f) const
            {
                return *this+date_time_period(f);
            }

            ///
            /// \brief subtract period f from the current date_time and return it
            ///
            date_time operator-(period::period_type f) const
            {
                return *this-date_time_period(f);
            }

            ///
            /// \brief add period f to the current date_time
            ///
            date_time const &operator+=(period::period_type f)
            {
                return *this+=date_time_period(f);
            }
            ///
            /// \brief subtract period f from the current date_time
            ///
            date_time const &operator-=(period::period_type f)
            {
                return *this-=date_time_period(f);
            }

            ///
            /// \brief roll up a date by period f.
            ///
            /// If overflow occures no other periods changed. i.e.:
            /// date_time(1990*year + 0 * month + 31 * day) << day == date_time(1990*year + 0 * month + 1 * day)
            ///
            date_time operator<<(period::period_type f) const
            {
                return *this<<date_time_period(f);
            }

            ///
            /// \brief roll down a date by period f, If overflow occures no other periods changed. i.e.:
            /// date_time(1990*year + 0 * month + 1 * day) >> day == date_time(1990*year + 0 * month + 31 * day)
            ///
            date_time operator>>(period::period_type f) const
            {
                return *this>>date_time_period(f);
            }

            ///
            /// \brief roll up a date by period f, If overflow occures no other periods changed. i.e.:
            /// date_time(1990*year + 0 * month + 31 * day) << day == date_time(1990*year + 0 * month + 1 * day)
            ///
            date_time const &operator<<=(period::period_type f)
            {
                return *this<<=date_time_period(f);
            }
            ///
            /// \brief roll down a date by period f, If overflow occures no other periods changed. i.e.:
            /// date_time(1990*year + 0 * month + 1 * day) >> day == date_time(1990*year + 0 * month + 31 * day)
            ///
            date_time const &operator>>=(period::period_type f)
            {
                return *this>>=date_time_period(f);
            }


            date_time operator+(date_time_period const &v) const;
            date_time operator-(date_time_period const &v) const;
            date_time const &operator+=(date_time_period const &v);
            date_time const &operator-=(date_time_period const &v);

            date_time operator<<(date_time_period const &v) const;
            date_time operator>>(date_time_period const &v) const ;
            date_time const &operator<<=(date_time_period const &v);
            date_time const &operator>>=(date_time_period const &v);

            date_time operator+(date_time_period_set const &v) const;
            date_time operator-(date_time_period_set const &v) const;
            date_time const &operator+=(date_time_period_set const &v);
            date_time const &operator-=(date_time_period_set const &v);

            date_time operator<<(date_time_period_set const &v) const;
            date_time operator>>(date_time_period_set const &v) const ;
            date_time const &operator<<=(date_time_period_set const &v);
            date_time const &operator>>=(date_time_period_set const &v);

            ///
            /// \brief Get POSIX time
            ///
            double time() const;
            ///
            /// \brief set POSIX time
            ///
            void time(double v);

            ///
            /// \brief compare date_time in the timeline (ingnores difference in calendar, timezone etc)
            ///
            bool operator==(date_time const &other) const;
            ///
            /// \brief compare date_time in the timeline (ingnores difference in calendar, timezone etc)
            ///
            bool operator!=(date_time const &other) const;
            ///
            /// \brief compare date_time in the timeline (ingnores difference in calendar, timezone etc)
            ///
            bool operator<(date_time const &other) const;
            ///
            /// \brief compare date_time in the timeline (ingnores difference in calendar, timezone etc)
            ///
            bool operator>(date_time const &other) const;
            ///
            /// \brief compare date_time in the timeline (ingnores difference in calendar, timezone etc)
            ///
            bool operator<=(date_time const &other) const;
            ///
            /// \brief compare date_time in the timeline (ingnores difference in calendar, timezone etc)
            ///
            bool operator>=(date_time const &other) const;

            ///
            /// \brief swaps to dates -- efficient
            ///
            void swap(date_time &other);

            ///
            /// \brief calculate the distance from this date_time to \a other in terms of perios \a f
            ///
            int difference(date_time const &other,period::period_type f) const;

            ///
            /// \brief Get minimal possible value for current time point for a period \a f.
            ///
            int minimum(period::period_type f) const;
            ///
            /// \brief Get minimal possible value for current time point for a period \a f. For example
            /// in February maximum(day) may be 28 or 29, in January maximum(day)==31
            ///
            int maximum(period::period_type f) const;

        private:
            void *impl_;
        };

        template<typename CharType>
        std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out,date_time const &t)
        {
            out << t.time();
            return out;
        }

        template<typename CharType>
        std::basic_istream<CharType> &operator>>(std::basic_istream<CharType> &in,date_time &t)
        {
            double v;
            in >> v;
            t.time(v);
            return in;
        }

        class date_time_duration {
        public:
            date_time_duration(date_time const &first,date_time const &second) :
                s_(first),
                e_(second)
            {
            }

            int operator / (period::period_type f) const
            {
                return start().difference(end(),f);
            }

            date_time const &start() const { return s_; }
            date_time const &end() const { return e_; }
        private:
            date_time s_,e_;
        };

        inline date_time_duration operator-(date_time const &later,date_time const &earlier)
        {
            return date_time_duration(earlier,later);
        }

        /// @}

    } // locale
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
