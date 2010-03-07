//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_DATE_TIME_HPP_INCLUDED
#define BOOST_LOCALE_DATE_TIME_HPP_INCLUDED

#include <boost/locale/timezone.hpp>
#include <locale>
#include <vector>
#include <stdexcept>

namespace boost {
    namespace locale {
        namespace date_time {
            
            class date_time_error : public std::runtime_error {
            public:
                date_time_error(std::string const &e) : std::runtime_error(e) {}
            };


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
            } field_type;


            struct date_time_field 
            {
                field_type field;
                int value;
                date_time_field operator+() const { return *this; }
                date_time_field operator-() const { return date_time_field(field,-value); }
                
                date_time_field(field_type f=invalid,int v=1) : field(f), value(v) {}
            };

            inline date_time_field operator+(field_type f) 
            {
                return date_time_field(f);
            }
            inline date_time_field operator-(field_type f)
            {
                return date_time_field(f,-1);
            }

            #define BOOST_LOCALE_DATE_TIME_OPERATORS(type) \
                inline date_time_field operator*(field_type f,type v) { return date_time_field(f,v); } \
                inline date_time_field operator*(type v,field_type f) { return date_time_field(f,v); } \
                inline date_time_field operator*(type v,date_time_field f) { return date_time_field(f.field,f.value*v); } \
                inline date_time_field operator*(date_time_field f,type v) { return date_time_field(f.field,f.value*v); }

            BOOST_LOCALE_DATE_TIME_OPERATORS(char)
            BOOST_LOCALE_DATE_TIME_OPERATORS(short int)
            BOOST_LOCALE_DATE_TIME_OPERATORS(int)
            BOOST_LOCALE_DATE_TIME_OPERATORS(long int)
            BOOST_LOCALE_DATE_TIME_OPERATORS(unsigned char)
            BOOST_LOCALE_DATE_TIME_OPERATORS(unsigned short int)
            BOOST_LOCALE_DATE_TIME_OPERATORS(unsigned int)
            BOOST_LOCALE_DATE_TIME_OPERATORS(unsigned long int)

            #undef BOOST_LOCALE_DATE_TIME_OPERATORS

            class date_time_field_set {
            public:
                date_time_field_set()
                {
                }
                date_time_field_set(field_type f)
                {
                    basic_=date_time_field(f);
                }
                date_time_field_set(date_time_field const &fl)
                {
                    basic_=fl;
                }
                void add(date_time_field f)
                {
                    if(basic_.field==invalid)
                        basic_=f;
                    else
                        fields_.push_back(f);
                }
                size_t size() const
                {
                    return basic_.field==invalid ? 0 : 1 + fields_.size();
                }
                date_time_field const &operator[](int n) const 
                {
                    if(n==0)
                        return basic_;
                    else
                        return fields_[n-1]; 
                }
            private:
                date_time_field basic_;
                std::vector<date_time_field> fields_;
            };

            inline date_time_field_set operator+(date_time_field_set const &a,date_time_field_set const &b)
            {
                date_time_field_set s(a);
                for(unsigned i=0;i<b.size();i++)
                    s.add(b[i]);
                return s;
            }
            
            inline date_time_field_set operator-(date_time_field_set const &a,date_time_field_set const &b)
            {
                date_time_field_set s(a);
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

                int minimum(field_type f) const;
                int greatest_minimum(field_type f) const;
                int maximum(field_type f) const;
                int least_maximum(field_type f) const;
                int first_day_of_week() const;

                std::locale get_locale() const;
                time_zone get_time_zone() const;

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
                date_time const &operator=(date_time const &other);
                ~date_time();

                date_time(double time);
                date_time(double time,calendar const &cal);
                date_time(calendar const &cal);
                
                date_time(date_time_field_set const &set);
                date_time(date_time_field_set const &set,calendar const &cal);

                
                date_time const &operator=(date_time_field_set const &f);

                void set(field_type f,int v);
                int get(field_type f) const;

                date_time operator+(field_type f) const
                {
                    return *this+date_time_field(f);
                }

                date_time operator-(field_type f) const
                {
                    return *this-date_time_field(f);
                }

                date_time const &operator+=(field_type f)
                {
                    return *this+=date_time_field(f);
                }
                date_time const &operator-=(field_type f)
                {
                    return *this-=date_time_field(f);
                }

                date_time operator<<(field_type f) const
                {
                    return *this<<date_time_field(f);
                }

                date_time operator>>(field_type f) const
                {
                    return *this>>date_time_field(f);
                }

                date_time const &operator<<=(field_type f)
                {
                    return *this<<=date_time_field(f);
                }
                date_time const &operator>>=(field_type f)
                {
                    return *this>>=date_time_field(f);
                }




                date_time operator+(date_time_field const &v) const;
                date_time operator-(date_time_field const &v) const;
                date_time const &operator+=(date_time_field const &v);
                date_time const &operator-=(date_time_field const &v);

                date_time operator<<(date_time_field const &v) const;
                date_time operator>>(date_time_field const &v) const ;
                date_time const &operator<<=(date_time_field const &v);
                date_time const &operator>>=(date_time_field const &v);

                date_time operator+(date_time_field_set const &v) const;
                date_time operator-(date_time_field_set const &v) const;
                date_time const &operator+=(date_time_field_set const &v);
                date_time const &operator-=(date_time_field_set const &v);

                date_time operator<<(date_time_field_set const &v) const;
                date_time operator>>(date_time_field_set const &v) const ;
                date_time const &operator<<=(date_time_field_set const &v);
                date_time const &operator>>=(date_time_field_set const &v);

                double time() const;
                void time(double v);

                bool operator==(date_time const &other) const;
                bool operator!=(date_time const &other) const;
                bool operator<(date_time const &other) const;
                bool operator>(date_time const &other) const;
                bool operator<=(date_time const &other) const;
                bool operator>=(date_time const &other) const;

                void swap(date_time &other);

                int difference(date_time const &other,field_type f) const;

                int minimum(field_type f) const;
                int maximum(field_type f) const;

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
                calendar cal(in);
                date_time tmp(cal);
                double v;
                in >> v;
                tmp.time(v);
                t.swap(tmp);
                return in;
            }

            class date_time_duration {
            public:
                date_time_duration(date_time const &first,date_time const &second) :
                    s_(first),
                    e_(second)
                {
                }

                int operator / (field_type f) const
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

            

/*
            template<fields::field_type Field>
            struct date_time_iterator : public std::random_access_iterator_tag<date_time> {
                typedef fields::basic_field<Field> field_type;

                date_time_iterator() {}
                date_time_iterator(date_time p) : dt_(p) {}
                date_time const &operator*() const { return dt_; }
                date_time &operator*() { return dt_; }
                date_time const *operator->() const { return &dt_; }
                date_time *operator->() { return &dt_; }

                date_time_iterator &operator++()
                {
                    dt_+=fields::basic_field<Field>(1);
                    return *this;
                }

                date_time_iterator &operator--()
                {
                    dt_+=fields::basic_field<Field>(-1);
                    return *this;
                }

                date_time_iterator operator++(int unused)
                {
                    date_time dt(dt_);
                    dt_+=fields::basic_field<Field>(1);
                    return dt;
                }

                date_time_iterator operator--(int unused)
                {
                    date_time dt(dt_);
                    dt_+=fields::basic_field<Field>(-1);
                    return dt;
                }

                template<fields::field_type F>
                bool operator < (date_time_iterator<F> const &other)
                {
                    return dt_ < other.dt_;
                }

                template<fields::field_type F>
                bool operator > (date_time_iterator<F> const &other)
                {
                    return dt_ > other.dt_;
                }
                
                template<fields::field_type F>
                bool operator >= (date_time_iterator<F> const &other)
                {
                    return dt_ >= other.dt_;
                }
                
                template<fields::field_type F>
                bool operator <= (date_time_iterator<F> const &other)
                {
                    return dt_ <= other.dt_;
                }

                template<fields::field_type F>
                bool operator == (date_time_iterator<F> const &other)
                {
                    return dt_ == other.dt_;
                }

                template<fields::field_type F>
                bool operator != (date_time_iterator<F> const &other)
                {
                    return dt_ != other.dt_;
                }

                date_time_iterator operator + (field_type const &v) const
                {
                    date_time_iterator it(*this);
                    it->dt_+=v;
                    return it;
                }
                
                date_time_iterator operator - (field_type const &v) const
                {
                    date_time_iterator it(*this);
                    it->dt_-=v;
                    return it;
                }

                field_type operator-(date_time_iterator const &other) const
                {
                    return field_type(date_time_duration(dt_,other.dt_));
                }


            private:
                date_time dt_;
            };*/

        } // date_time
    } // locale
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
