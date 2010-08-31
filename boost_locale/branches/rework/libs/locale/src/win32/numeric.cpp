//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_LOCALE_SOURCE
#include <locale>
#include <string>
#include <ios>
#include <boost/locale/formatting.hpp>
#include <boost/locale/generator.hpp>
#include <boost/locale/encoding.hpp>
#include <boost/shared_ptr.hpp>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <wctype.h>
#include <ctype.h>
#include <langinfo.h>
#include <monetary.h>

#include "all_generator.hpp"
#include "api.hpp"

namespace boost {
namespace locale {
namespace impl_win {
    namespace {

        std::ostreambuf_iterator<wchar_t> write_it(std::ostreambuf_iterator<wchar_t> out,std::wstring const &s)
        {
            for(size_t i=0;i<s.size();i++)
                *out++ = s[i];
            return out;
        }
        
        std::ostreambuf_iterator<char> write_it(std::ostreambuf_iterator<char> out,std::wstring const &s)
        {
            std::string tmp = conv::from_utf(s,"UTF-8");
            for(size_t i=0;i<tmp.size();i++)
                *out++ = tmp[i];
            return out;
        }
    }


template<typename CharType>
class num_format : public std::num_put<CharType>
{
public:
    typedef typename std::num_put<CharType>::iter_type iter_type;
    typedef std::basic_string<CharType> string_type;
    typedef CharType char_type;

    num_format(winlocale const &lc,size_t refs = 0) : 
        std::num_put<CharType>(refs),
        lc_(lc)
    {
    }
protected: 
    

    virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, long val) const
    {
        return do_real_put(out,ios,fill,val);
    }
    virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, unsigned long val) const
    {
        return do_real_put(out,ios,fill,val);
    }
    virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, double val) const
    {
        return do_real_put(out,ios,fill,val);
    }
    virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, long double val) const
    {
        return do_real_put(out,ios,fill,val);
    }
    
    #ifndef BOOST_NO_LONG_LONG 
    virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, long long val) const
    {
        return do_real_put(out,ios,fill,val);
    }
    virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, unsigned long long val) const
    {
        return do_real_put(out,ios,fill,val);
    }
    #endif


private:



    template<typename ValueType>
    iter_type do_real_put (iter_type out, std::ios_base &ios, char_type fill, ValueType val) const
    {
        typedef std::num_put<char_type> super;

        ios_info &info=ios_info::get(ios);

        switch(info.display_flags()) {
        case flags::posix:
            {
                std::stringstream ss;
                ss.imbue(std::locale::classic());
                return super::do_put(out,ss,fill,val);
            }
        case flags::date:
            return format_time(out,ios,fill,static_cast<time_t>(val),'x');
        case flags::time:
            return format_time(out,ios,fill,static_cast<time_t>(val),'X');
        case flags::datetime:
            return format_time(out,ios,fill,static_cast<time_t>(val),'c');
        case flags::strftime:
            return format_time(out,ios,fill,static_cast<time_t>(val),info.date_time_pattern<char_type>());
        case flags::currency:
            {
                if(info.currency_flags()==flags::currency_default || info.currency_flags() == flags::currency_national)
                    return format_currency(false,out,ios,fill,static_cast<long double>(val));
                else
                    return format_currency(true,out,ios,fill,static_cast<long double>(val));
            }

        case flags::number:
        case flags::percent:
        case flags::spellout:
        case flags::ordinal:
        default:
            return super::do_put(out,ios,fill,val);
        }
    }

    iter_type format_currency(bool /*intl*/,iter_type out,std::ios_base &ios,char_type fill,long double val) const
    {
        if(lc_.is_c()) {
            std::locale loc = ios.getloc();
            int digits = std::use_facet<std::moneypunct<char_type> >(loc).frac_digits();
            while(digits > 0) {
                val*=10;
                digits --;
            }
            std::ios_base::fmtflags f=ios.flags();
            ios.flags(f | std::ios_base::showbase);
            out = std::use_facet<std::money_put<char_type> >(loc).put(out,false,ios,fill,val);
            ios.flags(f);
            return out;
        }
        else {
            std::wstring cur = wcsfmon_l(val,lc_);
            return write_it(out,cur);
        }
    }

    iter_type format_time(iter_type out,std::ios_base &ios,char_type fill,time_t time,char c) const
    {
        string_type fmt;
        fmt+=char_type('%');
        fmt+=char_type(c);
        return format_time(out,ios,fill,time,fmt);
    }

    iter_type format_time(iter_type out,std::ios_base &ios,char_type fill,time_t time,string_type const &format) const
    {
        std::string tz = ios_info::get(ios).time_zone();
        std::tm tm;
        if(tz.empty()) {
            #ifdef BOOST_WINDOWS
            /// Windows uses TLS
            tm = *localtime(&time);
            #else
            localtime_r(&time,&tm);
            #endif
        }
        else  {
            int gmtoff = parse_tz(tz);
            time+=gmtoff;
            #ifdef BOOST_WINDOWS
            /// Windows uses TLS
            tm = *gmtime(&time);
            #else
            gmtime_r(&time,&tm);
            #endif
            
            #if defined(__linux) || defined(__FreeBSD__) || defined(__APPLE__) 
            // These have extra fields to specify timezone
            if(gmtoff!=0) {
                tm.tm_zone=tz.c_str();
                tm.tm_gmtoff = gmtoff;
            }
            #endif
        }
        return std::use_facet<std::time_put<char_type> >(ios.getloc()).put(out,ios,fill,&tm,format.c_str(),format.c_str()+format.size());
    }

    int parse_tz(std::string const &tz) const
    {
        int gmtoff = 0;
        std::string ltz;
        for(unsigned i=0;i<tz.size();i++) {
            if('a' <= tz[i] && tz[i] <= 'z')
                ltz += tz[i]-'a' + 'A';
            else if(tz[i]==' ')
                ;
            else
                ltz+=tz[i];
        }
        if(ltz.compare(0,3,"GMT")!=0 && ltz.compare(0,3,"UTC")!=0)
            return 0;
        if(ltz.size()<=3)
            return 0;
        char const *begin = ltz.c_str()+3;
        char *end=0;
        int hours = strtol(begin,&end,10);
        if(end != begin) {
            gmtoff+=hours * 3600;
        }
        if(*end==':') {
            begin=end+1;
            int minutes = strtol(begin,&end,10);
            if(end!=begin)
                gmtoff+=minutes * 60;
        }
        return gmtoff;
    }
private:
    winlocale lc_;

};  /// num_format


template<typename CharType>
class num_parse : public std::num_get<CharType>
{
public:
    num_parse(size_t refs = 0) : 
        std::num_get<CharType>(refs)
    {
    }
protected: 
    typedef typename std::num_get<CharType>::iter_type iter_type;
    typedef std::basic_string<CharType> string_type;
    typedef CharType char_type;

    virtual iter_type do_get(iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,long &val) const
    {
        return do_real_get(in,end,ios,err,val);
    }

    virtual iter_type do_get(iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,unsigned short &val) const
    {
        return do_real_get(in,end,ios,err,val);
    }

    virtual iter_type do_get(iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,unsigned int &val) const
    {
        return do_real_get(in,end,ios,err,val);
    }

    virtual iter_type do_get(iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,unsigned long &val) const
    {
        return do_real_get(in,end,ios,err,val);
    }

    virtual iter_type do_get(iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,float &val) const
    {
        return do_real_get(in,end,ios,err,val);
    }

    virtual iter_type do_get(iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,double &val) const
    {
        return do_real_get(in,end,ios,err,val);
    }

    virtual iter_type do_get (iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,long double &val) const
    {
        return do_real_get(in,end,ios,err,val);
    }

    #ifndef BOOST_NO_LONG_LONG 
    virtual iter_type do_get (iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,long long &val) const
    {
        return do_real_get(in,end,ios,err,val);
    }

    virtual iter_type do_get (iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,unsigned long long &val) const
    {
        return do_real_get(in,end,ios,err,val);
    }

    #endif

private:
    
    template<typename ValueType>
    iter_type do_real_get(iter_type in,iter_type end,std::ios_base &ios,std::ios_base::iostate &err,ValueType &val) const
    {
        typedef std::num_get<char_type> super;

        ios_info &info=ios_info::get(ios);

        switch(info.display_flags()) {
        case flags::posix:
            {
                std::stringstream ss;
                ss.imbue(std::locale::classic());
                return super::do_get(in,end,ss,err,val);
            }
        // date-time parsing is not supported
        // due to buggy standard
        case flags::date:
        case flags::time:
        case flags::datetime:
        case flags::strftime:

        case flags::number:
        case flags::percent:
        case flags::spellout:
        case flags::ordinal:
        
        case flags::currency:
        default:
            return super::do_get(in,end,ios,err,val);
        }
    }

};

template<typename CharType>
class time_put_win : public std::time_put<CharType> {
public:
    time_put_win(winlocale const &lc, size_t refs = 0) : 
        std::time_put<CharType>(refs),
        lc_(lc)
    {
    }
    virtual ~time_put_win()
    {
    }
    typedef typename std::time_put<CharType>::iter_type iter_type;
    typedef CharType char_type;
    typedef std::basic_string<char_type> string_type;

    virtual iter_type do_put(iter_type out,std::ios_base &ios,CharType fill,std::tm const *tm,char format,char modifier) const
    {
        return write_it(out,wcsftime_l(format,tm,lc_));
    }

private:
    winlocale lc_;
};


template<typename CharType>
class num_punct_win : public std::numpunct<CharType> {
public:
    typedef std::basic_string<CharType> string_type;
    num_punct_win(winlocale const &lc,size_t refs = 0) : 
        std::numpunct<CharType>(refs)
    {
        numeric_info np = wcsnumformat_l(lc) ;
        if(sizeof(CharType) == 1 && np.thousands_sep == L"\xA0")
            np.thousands_sep=L" ";

        to_str(np.thousands_sep,thousands_sep_);
        to_str(np.decimal_point,decimal_point_);
        grouping_ = np.grouping;
        if(thousands_sep_.size() > 1)
            grouping_ = std::string();
        if(decimal_point_.size() > 1)
            decimal_point_ = CharType('.');
    }
    
    void to_str(std::wstring &s1,std::wstring &s2)
    {
        s2.swap(s1);
    }

    void to_str(std::wstring &s1,std::string &s2)
    {
        s2=conv::from_utf(s1,"UTF-8");
    }
    virtual CharType do_decimal_point() const
    {
        return *decimal_point_.c_str();
    }
    virtual CharType do_thousands_sep() const
    {
        return *thousands_sep_.c_str();
    }
    virtual std::string do_grouping() const
    {
        return grouping_;
    }
    virtual string_type do_truename() const
    {
        static const char t[]="true";
        return string_type(t,t+sizeof(t)-1);
    }
    virtual string_type do_falsename() const
    {
        static const char t[]="false";
        return string_type(t,t+sizeof(t)-1);
    }
private:
    string_type decimal_point_;
    string_type thousands_sep_;
    std::string grouping_;
};

template<typename CharType>
std::locale create_formatting_impl(std::locale const &in,winlocale const &lc)
{
    if(lc.is_c()) {
        std::locale tmp(in,new std::numpunct_byname<CharType>("C"));
        tmp=std::locale(tmp,new std::time_put_byname<CharType>("C"));
        tmp = std::locale(tmp,new num_format<CharType>(lc));
        return tmp;
    }
    else {
        std::locale tmp(in,new num_punct_win<CharType>(lc));
        tmp = std::locale(tmp,new time_put_win<CharType>(lc));
        tmp = std::locale(tmp,new num_format<CharType>(lc));
        return tmp;
    }
}

template<typename CharType>
std::locale create_parsing_impl(std::locale const &in,winlocale const &lc)
{
    if(lc.is_c()) {
        std::locale tmp(in,new std::numpunct_byname<CharType>("C"));
        tmp = std::locale(tmp,new num_parse<CharType>());
        return tmp;
    }
    else {
        std::locale tmp = std::locale(in,new num_punct_win<CharType>(lc));
        tmp = std::locale(tmp,new num_parse<CharType>());
        return tmp;
    }
}


std::locale create_formatting(  std::locale const &in,
                                winlocale const &lc,
                                character_facet_type type)
{
        switch(type) {
        case char_facet: 
            return create_formatting_impl<char>(in,lc);
        case wchar_t_facet:
            return create_formatting_impl<wchar_t>(in,lc);
        default:
            return in;
        }
}

std::locale create_parsing( std::locale const &in,
                            winlocale const &lc,
                            character_facet_type type)
{
        switch(type) {
        case char_facet: 
            return create_parsing_impl<char>(in,lc);
        case wchar_t_facet:
            return create_parsing_impl<wchar_t>(in,lc);
        default:
            return in;
        }
}



} // impl_std
} // locale 
} //boost



// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
