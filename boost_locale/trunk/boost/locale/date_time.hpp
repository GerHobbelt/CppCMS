//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
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
        class date_time_error : public std::runtime_error {
        public:
            date_time_error(std::string const &e) : std::runtime_error(e) {}
        };

        namespace period {
            typedef enum {
                invalid,
                era,
                year,
                extended_year,
                month,
                day,
                day_of_year,
                day_of_week,
                day_of_week_in_month,
                day_of_week_local,
                hour,
                hour_12,
                am_pm,
                minute,
                second,
                week_of_year,
                week_of_month,
            } period_type;
        } // period

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

        class BOOST_LOCALE_DECL calendar {
        public:

            calendar(std::ios_base &ios);
            calendar(std::locale const &l,time_zone const &zone);
            calendar(std::locale const &l);
            calendar(time_zone const &zone);
            calendar();
            ~calendar();

            calendar(calendar const &other);
            calendar const &operator=(calendar const &other);

            int minimum(period::period_type f) const;
            int greatest_minimum(period::period_type f) const;
            int maximum(period::period_type f) const;
            int least_maximum(period::period_type f) const;
            int first_day_of_week() const;

            std::locale get_locale() const;
            time_zone get_time_zone() const;

            bool is_gregorian() const;

            bool operator==(calendar const &other) const;
            bool operator!=(calendar const &other) const;

        private:
            friend class date_time;
            std::locale locale_;
            boost::locale::time_zone tz_;
            void *impl_;
        };

        
        class BOOST_LOCALE_DECL date_time {
        public:

            date_time();
            date_time(date_time const &other);
            date_time(date_time const &other,date_time_period_set const &set);
            date_time const &operator=(date_time const &other);
            ~date_time();

            date_time(double time);
            date_time(double time,calendar const &cal);
            date_time(calendar const &cal);
            
            date_time(date_time_period_set const &set);
            date_time(date_time_period_set const &set,calendar const &cal);

            
            date_time const &operator=(date_time_period_set const &f);

            void set(period::period_type f,int v);
            int get(period::period_type f) const;

            date_time operator+(period::period_type f) const
            {
                return *this+date_time_period(f);
            }

            date_time operator-(period::period_type f) const
            {
                return *this-date_time_period(f);
            }

            date_time const &operator+=(period::period_type f)
            {
                return *this+=date_time_period(f);
            }
            date_time const &operator-=(period::period_type f)
            {
                return *this-=date_time_period(f);
            }

            date_time operator<<(period::period_type f) const
            {
                return *this<<date_time_period(f);
            }

            date_time operator>>(period::period_type f) const
            {
                return *this>>date_time_period(f);
            }

            date_time const &operator<<=(period::period_type f)
            {
                return *this<<=date_time_period(f);
            }
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

            double time() const;
            void time(double v);

            bool operator==(date_time const &other) const;
            bool operator!=(date_time const &other) const;
            bool operator<(date_time const &other) const;
            bool operator>(date_time const &other) const;
            bool operator<=(date_time const &other) const;
            bool operator>=(date_time const &other) const;

            void swap(date_time &other);

            int difference(date_time const &other,period::period_type f) const;

            int minimum(period::period_type f) const;
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

    } // locale
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
