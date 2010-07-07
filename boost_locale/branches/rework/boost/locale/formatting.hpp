//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_FORMATTING_HPP_INCLUDED
#define BOOST_LOCALE_FORMATTING_HPP_INCLUDED

#include <boost/locale/config.hpp>
#ifdef BOOST_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4275 4251 4231 4660)
#endif
#include <boost/locale/time_zone.hpp>
#include <boost/cstdint.hpp>
#include <ostream>
#include <istream>
#include <string>

namespace boost {
    namespace locale {
        namespace flags {
            typedef enum {
                posix               = 0,
                number              = 1,
                currency            = 2,
                percent             = 3,
                date                = 4,
                time                = 5,
                datetime            = 6,
                strftime            = 7,
                spellout            = 8,
                ordinal             = 9,

                display_flags_mask  = 31,

                currency_default    = 0 << 5,
                currency_iso        = 1 << 5,
                currency_national   = 2 << 5,

                currency_flags_mask = 3 << 5,

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

                datetime_flags_mask = date_flags_mask | time_flags_mask

            } display_flags_type;

            typedef enum {
                datetime_pattern,
                time_zone_id
            } pattern_type;

            typedef enum {
                domain_id
            } value_type;

            
        } // flags

        /// \cond INTERNAL

        class ios_info {
        public:

            class cached_info {
                cached_info(cached_info const &);
                void operator=(cached_info const &);
            public:
                cached_info(){}
                virtual ~cached_info(){}
                virtual cached_info *clone() = 0;
            };

            ios_info();
            ios_info(ios_info const &);
            void operator=(ios_info const &);
            ~ios_info();

            void cache(cached_info *info);
            cached_info *cache(std::typeinfo const &) const;

            template<typename T>
            T *cache() const
            {
                return dynamic_cast<T>(cache(typeid(T)));
            }

            static ios_info &get(std::ios_base &ios);

            void display_flags(uint64_t flags);
            
            void date_flags(uint64_t flags);
            
            void time_flags(uint64_t flags);
            
            void datetime_flags(uint64_t flags);
            
            void domain_id(int);
            
            void time_zone(std::string const &);
            
            template<typename CharType>
            void date_time_pattern(std::basic_string<CharType> const &str)
            {
                date_time_pattern_set().set<CharType>(str.c_str());
            }


            uint64_t display_flags() const;
            
            uint64_t date_flags() const;
            
            uint64_t time_flags() const;
            
            uint64_t datetime_flags() const;
            
            int domain_id() const;
            
            std::string time_zone() const;
            
            template<typename CharType>
            std::basic_string<CharType> date_time_pattern() const
            {
                return date_time_pattern_set().get<CharType>();
            }

            
        private:

            string_set const &date_time_pattern_set() const;
            string_set &date_time_pattern_set();
            
            class string_set {
            public:
                string_set() : 
                    type(0),
                    size(0),
                    ptr(0)
                {
                    string_.nothing_=0;
                }

                ~string_set()
                {
                    delete [] ptr;
                }
                string_set(string_set const &other)
                {
                    if(other.ptr!=0) {
                        ptr=new char[other.size](0);
                        size=other.size;
                        type=other.type;
                        memcpy(ptr,other.ptr,size);
                    }
                    else {
                        ptr=0;
                        size=0;
                        type=0;
                    }
                }
                string_set const &operator=(string_set const &other)
                {
                    if(this!=&other) {
                        delete [] ptr;
                        ptr = 0;
                        size = 0;
                        type = 0;
                        if(other.ptr!=0) {
                            ptr=new char[other.size](0);
                            size=other.size;
                            type=other.type;
                            memcpy(ptr,other.ptr,size);
                        }
                    }
                    return *this;
                }

                template<typename Char>
                void set(std::basic_string<Char> const &s)
                {
                    set<Char>(s.c_str());
                }
                
                template<typename Char>
                void set(Char const *s)
                {
                    delete [] ptr;
                    ptr = 0;
                    type=&typeid(Char);
                    Char *end = s;
                    while(*end!=0) end++;
                    size = sizeof(Char)*(end - s+1)
                    ptr = new char[size](0);
                    memcpy(ptr,s,size);
                }
                template<typename Char>
                std::basic_string<Char> get() const
                {
                    if(type==0 || *type!=typeid(Char))
                        throw std::bad_cast();
                    std::basic_string<Char> result = reinterpret_cast<Char const *>(ptr);
                    return result;
                }

                template<>
                void set<char>(std::string const &s)
                {
                    clear();
                }

            private:
                std::typeinfo const *type;
                size_t size;
                char *ptr;
            };

            uint64_t flags_;
            int domain_id_;
            std::string time_zone_;
            string_set datetime_;

            struct data;
            data *d_;

        };


        ///
        /// \brief This namespace includes all manipulators that can be used on IO streams
        ///
        namespace as {
            ///
            /// \defgroup manipulators I/O Stream manipulators
            ///
            /// @{
            ///

            ///
            /// Format values with "POSIX" or "C"  locale. Note, if locale was created with additional non-classic locale then
            /// These numbers may be localized
            ///
            
            inline std::ios_base & posix(std::ios_base & ios)
            {
                ios_base::get(ios).display_flags(flags::posix);
                return ios;
            }

            ///
            /// Format a number. Note, unlike standard number formatting, integers would be treated like real numbers when std::fixed or
            /// std::scientific manipulators were applied
            ///
            inline std::ios_base & number(std::ios_base & ios)
            {
                ios_base::get(ios).display_flags(flags::number);
                return ios;
            }
            
            ///
            /// Format currency, number is treated like amount of money
            ///
            inline std::ios_base & currency(std::ios_base & ios)
            {
                ios_base::get(ios).display_flags(flags::currency);
                return ios;
            }
            
            ///
            /// Format percent, value 0.3 is treaded as 30%.
            ///
            inline std::ios_base & percent(std::ios_base & ios)
            {
                ios_base::get(ios).display_flags(flags::percent);
                return ios;
            }
            
            ///
            /// Format a date, number is treaded as POSIX time
            ///
            inline std::ios_base & date(std::ios_base & ios)
            {
                ios_base::get(ios).display_flags(flags::date);
                return ios;
            }

            ///
            /// Format a time, number is treaded as POSIX time
            ///
            inline std::ios_base & time(std::ios_base & ios)
            {
                ios_base::get(ios)->display_flags(flags::time);
                return ios;
            }

            ///
            /// Format a date and time, number is treaded as POSIX time
            ///
            inline std::ios_base & datetime(std::ios_base & ios)
            {
                ext_setf(ios, flags::datetime, flags::display_flags_mask);
                return ios;
            }

            ///
            /// Create formatted date time, Please note, this manipulator only changes formatting mode,
            /// and not format itself, so you are probably looking for ftime manipulator
            ///
            inline std::ios_base & strftime(std::ios_base & ios)
            {
                ext_setf(ios, flags::strftime, flags::display_flags_mask);
                return ios;
            }
            
            ///
            /// Spell the number, like "one hundred and ten"
            ///
            inline std::ios_base & spellout(std::ios_base & ios)
            {
                ext_setf(ios, flags::spellout, flags::display_flags_mask);
                return ios;
            }
            
            ///
            /// Write an order of the number like 4th.
            ///
            inline std::ios_base & ordinal(std::ios_base & ios)
            {
                ext_setf(ios, flags::ordinal, flags::display_flags_mask);
                return ios;
            }

            ///
            /// Set default currency formatting style -- national, like "$"
            ///
            inline std::ios_base & currency_default(std::ios_base & ios)
            {
                ext_setf(ios, flags::currency_default, flags::currency_flags_mask);
                return ios;
            }

            ///
            /// Set ISO currency formatting style, like "USD", (requires ICU >= 4.2)
            ///
            inline std::ios_base & currency_iso(std::ios_base & ios)
            {
                ext_setf(ios, flags::currency_iso, flags::currency_flags_mask);
                return ios;
            }

            ///
            /// Set national currency formatting style, like "$"
            ///
            inline std::ios_base & currency_national(std::ios_base & ios)
            {
                ext_setf(ios, flags::currency_national, flags::currency_flags_mask);
                return ios;
            }

            ///
            /// set default (medium) time formatting style
            ///
            inline std::ios_base & time_default(std::ios_base & ios)
            {
                ext_setf(ios, flags::time_default, flags::time_flags_mask);
                return ios;
            }

            ///
            /// set short time formatting style
            ///
            inline std::ios_base & time_short(std::ios_base & ios)
            {
                ext_setf(ios, flags::time_short, flags::time_flags_mask);
                return ios;
            }

            ///
            /// set medium time formatting style
            ///
            inline std::ios_base & time_medium(std::ios_base & ios)
            {
                ext_setf(ios, flags::time_medium, flags::time_flags_mask);
                return ios;
            }

            ///
            /// set long time formatting style
            ///
            inline std::ios_base & time_long(std::ios_base & ios)
            {
                ext_setf(ios, flags::time_long, flags::time_flags_mask);
                return ios;
            }

            ///
            /// set full time formatting style
            ///
            inline std::ios_base & time_full(std::ios_base & ios)
            {
                ext_setf(ios, flags::time_full, flags::time_flags_mask);
                return ios;
            }

            ///
            /// set default (medium) date formatting style
            ///
            inline std::ios_base & date_default(std::ios_base & ios)
            {
                ext_setf(ios, flags::date_default, flags::date_flags_mask);
                return ios;
            }

            ///
            /// set short date formatting style
            ///
            inline std::ios_base & date_short(std::ios_base & ios)
            {
                ext_setf(ios, flags::date_short, flags::date_flags_mask);
                return ios;
            }

            ///
            /// set medium date formatting style
            ///
            inline std::ios_base & date_medium(std::ios_base & ios)
            {
                ext_setf(ios, flags::date_medium, flags::date_flags_mask);
                return ios;
            }

            ///
            /// set long date formatting style
            ///
            inline std::ios_base & date_long(std::ios_base & ios)
            {
                ext_setf(ios, flags::date_long, flags::date_flags_mask);
                return ios;
            }

            ///
            /// set full date formatting style
            ///
            inline std::ios_base & date_full(std::ios_base & ios)
            {
                ext_setf(ios, flags::date_full, flags::date_flags_mask);
                return ios;
            }            
            
            
            /// \cond INTERNAL 
            namespace details {
                template<typename CharType>
                struct add_ftime {

                    std::basic_string<CharType> ftime;

                    void apply(std::basic_ios<CharType> &ios) const
                    {
                        ext_pattern(ios,flags::datetime_pattern,ftime);
                        as::strftime(ios);
                    }

                };

                template<typename CharType>
                std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out,add_ftime<CharType> const &fmt)
                {
                    fmt.apply(out);
                    return out;
                }
                
                template<typename CharType>
                std::basic_istream<CharType> &operator>>(std::basic_istream<CharType> &in,add_ftime<CharType> const &fmt)
                {
                    fmt.apply(in);
                    return in;
                }

            }
            /// \endcond 

            ///
            /// Set strftime like formatting string
            ///
            /// Please note, formatting flags are very similar but not exactly the same as flags for C function strftime.
            /// Differences: some flags as "%e" do not add blanks to fill text up to two spaces, not all flags supported.
            ///
            /// Flags:
            /// -   "%a" -- Abbreviated  weekday (Sun.)
            /// -   "%A" -- Full weekday (Sunday)
            /// -   "%b" -- Abbreviated month (Jan.)
            /// -   "%B" -- Full month (January)
            /// -   "%c" -- Locale date-time format. **Note:** prefer using "as::datetime"
            /// -   "%d" -- Day of Month [01,31]
            /// -   "%e" -- Day of Month [1,31]
            /// -   "%h" -- Same as "%b"
            /// -   "%H" -- 24 clock hour [00,23]
            /// -   "%I" -- 12 clock hour [01,12]
            /// -   "%j" -- Day of year [1,366]
            /// -   "%m" -- Month [01,12]
            /// -   "%M" -- Minute [00,59]
            /// -   "%n" -- New Line
            /// -   "%p" -- AM/PM in locale representation
            /// -   "%r" -- Time with AM/PM, same as "%I:%M:%S %p"
            /// -   "%R" -- Same as "%H:%M"
            /// -   "%S" -- Second [00,61]
            /// -   "%t" -- Tab character
            /// -   "%T" -- Same as "%H:%M:%S"
            /// -   "%x" -- Local date representation. **Note:** prefer using "as::date"
            /// -   "%X" -- Local time representation. **Note:** prefer using "as::time"
            /// -   "%y" -- Year [00,99]
            /// -   "%Y" -- 4 digits year. (2009)
            /// -   "%Z" -- Time Zone
            /// -   "%%" -- Percent symbol
            ///


            template<typename CharType>
            details::add_ftime<CharType> ftime(std::basic_string<CharType> const &format)
            {
                details::add_ftime<CharType> fmt;
                fmt.ftime=format;
                return fmt;
            }

            ///
            /// See ftime(std::basic_string<CharType> const &format)
            ///
            template<typename CharType>
            details::add_ftime<CharType> ftime(CharType const *format)
            {
                details::add_ftime<CharType> fmt;
                fmt.ftime=format;
                return fmt;
            }

            /// \cond INTERNAL
            namespace details {
                struct set_timezone {
                    std::string id;
                };
                template<typename CharType>
                std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out,set_timezone const &fmt)
                {
                    ext_pattern(out,flags::time_zone_id,fmt.id);
                    return out;
                }
                
                template<typename CharType>
                std::basic_istream<CharType> &operator>>(std::basic_istream<CharType> &in,set_timezone const &fmt)
                {
                    ext_pattern(in,flags::time_zone_id,fmt.id);
                    return in;
                }
            }
            /// \endcond
            
            ///
            /// Set GMT time zone to stream
            /// 
            inline std::ios_base &gmt(std::ios_base &ios)
            {
                ext_pattern<char>(ios,flags::time_zone_id,"GMT");
                return ios;
            }

            ///
            /// Set local time zone to stream
            ///
            inline std::ios_base &local_time(std::ios_base &ios)
            {
                ext_pattern(ios,flags::time_zone_id,std::string());
                return ios;
            }

            ///
            /// Set time zone using \a id
            ///
            inline details::set_timezone time_zone(char const *id) 
            {
                details::set_timezone tz;
                tz.id=id;
                return tz;
            }

            ///
            /// Set time zone using \a id
            ///
            inline details::set_timezone time_zone(std::string const &id) 
            {
                details::set_timezone tz;
                tz.id=id;
                return tz;
            }

            ///
            /// Set time zone using time_zone class \a id
            ///
            inline details::set_timezone time_zone(boost::locale::time_zone const &id) 
            {
                details::set_timezone tz;
                tz.id=id.id();
                return tz;
            }


        ///
        /// @}
        ///

        } // as manipulators
        
    } // locale
} // boost

#ifdef BOOST_MSVC
#pragma warning(pop)
#endif


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
