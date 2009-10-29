#define BOOST_LOCALE_SOURCE
#include <boost/locale/format.hpp>
#include <limits>
#include "formatting_info.hpp"

#include <iostream>

namespace boost {
    namespace locale {
        namespace details {
            struct format_parser::data {
                unsigned position;
                std::streamsize precision;
                std::ios_base::fmtflags flags;
                impl::ios_info info;
            };

            format_parser::format_parser(std::ios_base &ios) : 
                ios_(ios),
                d(new data)
            {
                d->position=std::numeric_limits<unsigned>::max();
                d->precision=ios.precision();
                d->info=impl::ios_prop<impl::ios_info>::get(ios);
            }

            format_parser::~format_parser()
            {
            }

            void format_parser::restore()
            {
                impl::ios_prop<impl::ios_info>::set(d->info,ios_);
                ios_.width(0);
            }

            unsigned format_parser::get_posision()
            {
                return d->position;
            }

            void format_parser::set_flags(std::string const &format)
            {
                size_t end = 0;
                for(size_t begin = 0;begin < format.size();begin = ( end  == std::string::npos ? end : end+1)) {
                        end=format.find(',',begin);
                        set_one_flag(format.substr(begin,end-begin));
                }                
            }
            void format_parser::set_one_flag(std::string const &flag)
            {
                size_t pos=flag.find('=');
                if(pos==std::string::npos)
                    set_one_flag(flag,"");
                else
                    set_one_flag(flag.substr(0,pos),flag.substr(pos+1));
            }
            void format_parser::set_one_flag(std::string const &key,std::string const &value)
            {
                if(key.empty())
                    return;
                unsigned i;
                for(i=0;i<key.size();i++) {
                    if(key[i] < '0' || '9'< key[i])
                        break;
                }
                if(i==key.size()) {
                    d->position=atoi(key.c_str()) - 1;
                    return;
                }

                if(key=="num" || key=="number") {
                    as::number(ios_);

                    if(value=="hex")
                        ios_.setf(std::ios_base::hex,std::ios_base::basefield);
                    else if(value=="oct")
                        ios_.setf(std::ios_base::oct,std::ios_base::basefield);
                    else if(value=="sci" || value=="scientific")
                        ios_.setf(std::ios_base::scientific,std::ios_base::floatfield);
                }
                else if(key=="cur" || key=="currency") {
                    as::currency(ios_);
                    if(value=="iso") 
                        as::currency_iso(ios_);
                    else if(value=="nat" || value=="national")
                        as::currency_national(ios_);
                }
                else if(key=="per" || key=="percent") {
                    as::percent(ios_);
                }
                else if(key=="date") {
                    as::date(ios_);
                    if(value=="s" || value=="short")
                        as::date_short(ios_);
                    else if(value=="m" || value=="medium")
                        as::date_medium(ios_);
                    else if(value=="l" || value=="long")
                        as::date_long(ios_);
                    else if(value=="f" || value=="full")
                        as::date_full(ios_);
                }
                else if(key=="time") {
                    as::time(ios_);
                    if(value=="s" || value=="short")
                        as::time_short(ios_);
                    else if(value=="m" || value=="medium")
                        as::time_medium(ios_);
                    else if(value=="l" || value=="long")
                        as::time_long(ios_);
                    else if(value=="f" || value=="full")
                        as::time_full(ios_);
                }
                else if(key=="dt" || key=="datetime") {
                    as::datetime(ios_);
                    if(value=="s" || value=="short") {
                        as::date_short(ios_);
                        as::time_short(ios_);
                    }
                    else if(value=="m" || value=="medium") {
                        as::date_medium(ios_);
                        as::time_medium(ios_);
                    }
                    else if(value=="l" || value=="long") {
                        as::date_long(ios_);
                        as::time_long(ios_);
                    }
                    else if(value=="f" || value=="full") {
                        as::date_full(ios_);
                        as::time_full(ios_);
                    }
                }
                else if(key=="spell" || key=="spellout") {
                    as::spellout(ios_);
                }
                else if(key=="ord" || key=="ordinal") {
                    as::ordinal(ios_);
                }
                else if(key=="left" || key=="<")
                    ios_.setf(std::ios_base::left,std::ios_base::adjustfield);
                else if(key=="right" || key==">")
                    ios_.setf(std::ios_base::right,std::ios_base::adjustfield);
                else if(key=="gmt")
                    as::gmt(ios_);
                else if(key=="local")
                    as::local_time(ios_);
                else if(key=="timezone" || key=="tz")
                    ext_pattern(ios_,flags::time_zone_id,value);
                else if(key=="w" || key=="width")
                    ios_.width(atoi(value.c_str()));
                else if(key=="p" || key=="precision")
                    ios_.precision(atoi(value.c_str()));

            }
        }
    }
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4