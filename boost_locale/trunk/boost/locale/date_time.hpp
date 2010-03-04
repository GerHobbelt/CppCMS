//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_DATE_TIME_HPP_INCLUDED
#define BOOST_LOCALE_DATE_TIME_HPP_INCLUDED

namespace boost {
    namespace locale {
        namespace date_time {

            
            class date_time_duration;

            namespace fields {

                typedef enum {
                    invalid,
                    ara,
                    year,
                    extended_year,
                    month,
                    month_is_leap,
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
                
                struct field_set {
                    typedef  std::pair<fields::field_type,int> element_type;
                    void add(fields::field_type type,int v) 
                    {
                        ext_.push_back(element_type(F,f()));
                    }
                    size_t size() const { return ext_.size(); }
                    element_type operator[](int n) const { return ext_.at(n); }
                private:
                    std::vector<element_type> ext_;
                };

                template<fileds_type field>
                struct basic_field {
                    static const field_type type=field;

                    basic_field(int v=0) : value_(v) {}

                    basic_field(date_time_duration const &d);

                    void value(int v) { value_ = v; }

                    int value() const { return value_; }

                    int operator() const { return value_; }

                    basic_field operator-() const { return basic_field(-value_); }

                    template<field_type F>
                    field_set operator+(basic_field<F> const &other) const
                    {
                        field_set s;
                        s.add(field,value_);
                        s.add(F,other.value_);
                    }

                private:
                    int value_;
                };

                template<field_type F>
                field_set &operator+(field_set &set,basic_field<F> const &other)
                {
                    set.add(F,other());
                    return set;
                }

            } // fields


            #define BOOST_LOCALE_FIELDS_DEF(name) typedef basic_field<fields::name> name
            BOOST_LOCALE_FIELDS_DEF(ara);
            BOOST_LOCALE_FIELDS_DEF(year);
            BOOST_LOCALE_FIELDS_DEF(extended_year);
            BOOST_LOCALE_FIELDS_DEF(month);
            BOOST_LOCALE_FIELDS_DEF(month_is_leap);
            BOOST_LOCALE_FIELDS_DEF(date);
            BOOST_LOCALE_FIELDS_DEF(day);
            BOOST_LOCALE_FIELDS_DEF(day_of_year);
            BOOST_LOCALE_FIELDS_DEF(day_of_week);
            BOOST_LOCALE_FIELDS_DEF(day_of_week_in_month);
            BOOST_LOCALE_FIELDS_DEF(day_of_week_local);
            BOOST_LOCALE_FIELDS_DEF(hour);
            BOOST_LOCALE_FIELDS_DEF(hour_12);
            BOOST_LOCALE_FIELDS_DEF(am_pm);
            BOOST_LOCALE_FIELDS_DEF(minute);
            BOOST_LOCALE_FIELDS_DEF(second);
            BOOST_LOCALE_FIELDS_DEF(week_of_year);
            BOOST_LOCALE_FIELDS_DEF(week_of_month);
            #undef BOOST_LOCALE_FIELDS_DEF


            class BOOST_LOCALE_DECL calendar {
            public:

                calendar(std::locale const &l,time_zone const &zone);
                calendar(std::locale const &l);
                calendar(time_zone const &zone);
                calendar();
                ~calendar();

                calendar(calendar const &other);
                calendar const &operator=(calendar const &other);

                boost::locale::time_zone time_zone() const
                std::locale locale() const;

                template<Field>
                Field minimum() const
                {
                    return Field(range(Field::type).first);
                }
                template<Field>
                Field greatest_minimum() const
                {
                    return Field(min_range(Field::type).first);
                }
                template<Field>
                Field maximum() const
                {
                    return Field(range(Field::type).second);
                }
                template<Field>
                Field least_maximum() const
                {
                    return Field(min_range(Field::type).second);
                }

                day_of_week first_day_of_week() const;

                std::pair<int,int> range(fields::field_type t) const;
                std::pair<int,int> min_range(fields::field_type t) const;

                bool operator==(calendar const &other) const;
                bool operator!=(calendar const &other) const
                {
                    return !(*this == other);
                }

            private:
                std::locale locale_;
                boost::locale::time_zone tz_;
                void *impl_;
            };

            
            class BOOST_LOCALE_DECL date_time {
            public:

                date_time(std::ios_base &ios);
                date_time();
                date_time(double time);
                date_time(double time,calendar const &cal);

                date_time(fields::field_set const &set)
                {
                    init();
                    for(unsigned i=0;i<set.size();i++)
                        set_field(set[i].first,set[i].second);
                }

                date_time(fields::field_set const &set,calendar const &cal)
                {
                    init(cal);
                    for(unsigned i=0;i<set.size();i++)
                        set_field(set[i].first,set[i].second);
                }

                ~date_time();

                template<fields::field_type F>
                date_time(fields::basic_field<F> f1)
                {
                    init();
                    set(f1);
                }
                
                template<fields::field_type F>
                date_time(fields::basic_field<F> f1,calendar const &cal)
                {
                    init(cal);
                    set(f1);
                }



                template<typename F1,typename F2>
                template<fields::field_type f>
                date_time const &operator=(fields::basic_field<f> v)
                {
                    set(f,v);
                }

                template<F>
                F get() const
                {
                    return F(get_field(F::type));
                }
                
                template<F>
                void set() const
                {
                    return set_field(F::type,F());
                }
                 
                template<fields::field_type f>
                date_time operator+(fields::basic_field<f> v) const
                {
                    date_time tmp(*this);
                    tmp.advance(f,v());
                    return tmp;
                }

                template<fields::field_type f>
                date_time operator-(fields::basic_field<f> v) const
                {
                    date_time tmp(*this);
                    tmp.advance(f,-v());
                    return tmp;
                }

                template<fields::field_type f>
                date_time const &operator+=(fields::basic_field<f> v)
                {
                    advance(f,v());
                    return *this;
                }

                template<fields::field_type f>
                date_time const &operator-=(fields::basic_field<f> v)
                {
                    advance(f,-v());
                    return *this;
                }

                double time() const;
                void time(double v);
            private:

            };

            template<typename CharType>
            std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out,date_time const &t)
            {
                out << t.time();
            }

            template<typename CharType>
            std::basic_istream<CharType> &operator>>(std::basic_istream<CharType> &in,date_time &t)
            {
                date_time tmp(in);
                double v;
                in >> v;
                tmp.time(v);
                t=tmp;
                return in;
            }
            


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
            };

            class date_time_duration {
            public:
                date_time_duration(date_time const &first,date_time const &second) :
                    first_(first),
                    second_(second)
                {
                }

                date_time const &begin() const { return first_; }
                date_time const &end() const { return second_; }
            private:
                date_time first_,second_;
            };

            namespace fields {
                template<field_type F>
                basic_field::basic_field(date_time_duration const &d) :
                    value_(d.begin().difference(F,d.end()))
                {
                }
            }
        }
    }
}

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
