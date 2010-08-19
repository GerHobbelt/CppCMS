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
#include <boost/locale/codepage.hpp>
#include <sstream>
#include <stdlib.h>


#include "all_generator.hpp"

namespace boost {
namespace locale {
namespace impl_std {

template<typename CharType>
class num_format : public std::num_put<CharType>
{
public:
    typedef typename std::num_put<CharType>::iter_type iter_type;
    typedef std::basic_string<CharType> string_type;
    typedef CharType char_type;

    num_format(size_t refs = 0) : 
        std::num_put<CharType>(refs)
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
                    return format_currency<false>(out,ios,fill,static_cast<long double>(val));
                else
                    return format_currency<true>(out,ios,fill,static_cast<long double>(val));
            }

        case flags::number:
        case flags::percent:
        case flags::spellout:
        case flags::ordinal:
        default:
            return super::do_put(out,ios,fill,val);
        }
    }

    template<bool intl>
    iter_type format_currency(iter_type out,std::ios_base &ios,char_type fill,long double val) const
    {
        std::locale loc = ios.getloc();
        int digits = std::use_facet<std::moneypunct<char_type,intl> >(loc).frac_digits();
        while(digits > 0) {
            val*=10;
            digits --;
        }
        std::ios_base::fmtflags f=ios.flags();
        ios.flags(f | std::ios_base::showbase);
        out = std::use_facet<std::money_put<char_type> >(loc).put(out,intl,ios,fill,val);
        ios.flags(f);
        return out;
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
        case flags::currency:
            {
                long double ret_val = 0;
                if(info.currency_flags()==flags::currency_default || info.currency_flags() == flags::currency_national)
                    in = parse_currency<false>(in,end,ios,err,ret_val);
                else
                    in = parse_currency<true>(in,end,ios,err,ret_val);
                if(!(err & std::ios_base::failbit))
                    val = static_cast<ValueType>(ret_val);
                return in;
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
        default:
            return super::do_get(in,end,ios,err,val);
        }
    }

    template<bool intl>
    iter_type parse_currency(iter_type in,iter_type end,std::ios_base &ios,std::ios_base::iostate &err,long double &val) const
    {
        std::locale loc = ios.getloc();
        int digits = std::use_facet<std::moneypunct<char_type,intl> >(loc).frac_digits();
        long double rval;
        in = std::use_facet<std::money_get<char_type> >(loc).get(in,end,intl,ios,err,rval);
        if(!(err & std::ios::failbit)) {
            while(digits > 0) {
                rval/=10;
                digits --;
            }
            val = rval;
        }
        return in;
    }


};

template<typename CharType> 
class time_put_impl : public std::time_put<CharType> {
public:
    typedef typename std::time_put<CharType>::iter_type iter_type;
    typedef typename CharType char_type;

    virtual iter_type do_put(iter_type out,std::ios_base &ios,CharType fill,std::tm const *tm,char format,char modifier) const
    {
        switch(format) {
        case 'a':
            return write(out,p_.abbr_day,tm->wday,7);
        case 'A':
            return write(out,p_.day,tm->tm_wday,7);
        case 'b':
            return write(out,p_.abbr_month,tm->tm_mon,12);
        case 'B':
            return write(out,p_.month,tm->tm_mon,12);
        case 'c':
            return put(out,ios,fill,tm,p_.date_time_fmt.c_str(),p_.date_time_fmt.c_str()+p_.date_time_fmt.size());
        }
    }
 
};

template<typename CharType>
struct time_punct {
    typedef std::basic_string<CharType> string_type;
    string_type month[12];
    string_type abbr_month[12];
    string_type day[7];
    string_type abbr_day[7];
    string_type am;
    string_type pm;
    string_type date_time_fmt;
    string_type time_fmt;
    string_type date_fmt;
    string_type ampm_fmt;
};



template<typename CharType>
class time_put_from_base : public std::time_put<CharType> {
public:
    time_put_from_base(std::locale const &base, size_t refs = 0) : 
        std::time_put<CharType>(refs),
        base_(base)
    {
    }
    typedef typename std::time_put<CharType>::iter_type iter_type;

    virtual iter_type do_put(iter_type out,std::ios_base &ios,CharType fill,std::tm const *tm,char format,char modifier) const
    {
        std::basic_stringstream<CharType> ss;
        ss.imbue(base_);
        return std::use_facet<std::time_put<CharType> >(base_).put(out,ss,fill,tm,format,modifier);
    }
private:
    std::locale base_;
};

#ifndef BOOST_NO_STD_WSTRING
class utf8_time_put_from_wide : public std::time_put<char> {
public:
    utf8_time_put_from_wide(std::locale const &base, size_t refs = 0) : 
        std::time_put<char>(refs),
        base_(base)
    {
    }
    virtual iter_type do_put(iter_type out,std::ios_base &ios,char fill,std::tm const *tm,char format,char modifier = 0) const
    {
        std::basic_ostringstream<wchar_t> wtmps;
        wtmps.imbue(base_);
        std::use_facet<std::time_put<wchar_t> >(base_).put(wtmps,wtmps,wchar_t(fill),tm,wchar_t(format),wchar_t(modifier));
        std::wstring wtmp=wtmps.str();
        std::string const tmp = conv::from_utf<wchar_t>(wtmp,"UTF-8");
        for(unsigned i=0;i<tmp.size();i++) {
            *out++ = tmp[i];
        }
        return out;
    }
private:
    std::locale base_;
};

class utf8_numpunct_from_wide : public std::numpunct<char> {
public:
    utf8_numpunct_from_wide(std::locale const &base,size_t refs = 0) : std::numpunct<char>(refs)
    {
        typedef std::numpunct<wchar_t> wfacet_type;
        wfacet_type const &wfacet = std::use_facet<wfacet_type>(base);
        
        truename_ = conv::from_utf<wchar_t>(wfacet.truename(),"UTF-8");
        falsename_ = conv::from_utf<wchar_t>(wfacet.falsename(),"UTF-8");
        
        wchar_t tmp_decimal_point = wfacet.decimal_point();
        wchar_t tmp_thousands_sep = wfacet.thousands_sep();
        std::string tmp_grouping = wfacet.grouping();
        
        if( 32 <= tmp_thousands_sep && tmp_thousands_sep <=126 &&
            32 <= tmp_decimal_point && tmp_decimal_point <=126)
        {
            thousands_sep_ = static_cast<char>(tmp_thousands_sep);
            decimal_point_ = static_cast<char>(tmp_decimal_point);
            grouping_ = tmp_grouping;
        }
        else if(32 <= tmp_decimal_point && tmp_decimal_point <=126 && tmp_thousands_sep == 0xA0) {
            // workaround common bug - substitute NBSP with ordinary space
            thousands_sep_ = ' ';
            decimal_point_ = static_cast<char>(tmp_decimal_point);
            grouping_ = tmp_grouping;
        }
        else if(32 <= tmp_decimal_point && tmp_decimal_point <=126)
        {
            thousands_sep_=',';
            decimal_point_ = static_cast<char>(tmp_decimal_point);
            grouping_=std::string();
        }
        else {
            thousands_sep_ = ',';
            decimal_point_ = '.';
            grouping_=std::string();
        }
    }

    virtual char do_decimal_point() const
    {
        return decimal_point_;
    }
    virtual char do_thousands_sep() const
    {
        return thousands_sep_;
    }
    virtual std::string do_grouping() const
    {
        return grouping_;
    }
    virtual std::string do_truename() const
    {
        return truename_;
    }
    virtual std::string do_falsename() const
    {
        return falsename_;
    }
private:
    std::string truename_;
    std::string falsename_;
    char thousands_sep_;
    char decimal_point_;
    std::string grouping_;
    
};

template<bool Intl>
class utf8_moneypunct_from_wide : public std::moneypunct<char,Intl> {
public:
    utf8_moneypunct_from_wide(std::locale const &base,size_t refs = 0) : std::moneypunct<char,Intl>(refs)
    {
        typedef std::moneypunct<wchar_t,Intl> wfacet_type;
        wfacet_type const &wfacet = std::use_facet<wfacet_type>(base);

        curr_symbol_ = conv::from_utf<wchar_t>(wfacet.curr_symbol(),"UTF-8");
        positive_sign_ = conv::from_utf<wchar_t>(wfacet.positive_sign(),"UTF-8");
        negative_sign_ = conv::from_utf<wchar_t>(wfacet.negative_sign(),"UTF-8");
        frac_digits_ = wfacet.frac_digits();
        pos_format_ = wfacet.pos_format();
        neg_format_ = wfacet.neg_format();

        wchar_t tmp_decimal_point = wfacet.decimal_point();
        wchar_t tmp_thousands_sep = wfacet.thousands_sep();
        std::string tmp_grouping = wfacet.grouping();
        if( 32 <= tmp_thousands_sep && tmp_thousands_sep <=126 &&
            32 <= tmp_decimal_point && tmp_decimal_point <=126)
        {
            thousands_sep_ = static_cast<char>(tmp_thousands_sep);
            decimal_point_ = static_cast<char>(tmp_decimal_point);
            grouping_ = tmp_grouping;
        }
        else if(32 <= tmp_decimal_point && tmp_decimal_point <=126 && tmp_thousands_sep == 0xA0) {
            // workaround common bug - substitute NBSP with ordinary space
            thousands_sep_ = ' ';
            decimal_point_ = static_cast<char>(tmp_decimal_point);
            grouping_ = tmp_grouping;
        }
        else if(32 <= tmp_decimal_point && tmp_decimal_point <=126)
        {
            thousands_sep_=',';
            decimal_point_ = static_cast<char>(tmp_decimal_point);
            grouping_=std::string();
        }
        else {
            thousands_sep_ = ',';
            decimal_point_ = '.';
            grouping_=std::string();
        }
    }

    virtual char do_decimal_point() const
    {
        return decimal_point_;
    }

    virtual char do_thousands_sep() const
    {
        return thousands_sep_;
    }

    virtual std::string do_grouping() const
    {
        return grouping_;
    }

    virtual std::string do_curr_symbol() const
    {
        return curr_symbol_;
    }
    virtual std::string do_positive_sign () const
    {
        return positive_sign_;
    }
    virtual std::string do_negative_sign() const
    {
        return negative_sign_;
    }

    virtual int do_frac_digits() const
    {
        return frac_digits_;
    }

    virtual std::money_base::pattern do_pos_format() const
    {
        return pos_format_;
    }

    virtual std::money_base::pattern do_neg_format() const
    {
        return neg_format_;
    }

private:
    char thousands_sep_;
    char decimal_point_;
    std::string grouping_;
    std::string curr_symbol_;
    std::string positive_sign_;
    std::string negative_sign_;
    int frac_digits_;
    std::money_base::pattern pos_format_,neg_format_;
    
};

class utf8_numpunct : public std::numpunct_byname<char> {
public:
    typedef std::numpunct_byname<char> base_type;
    utf8_numpunct(char const *name,size_t refs = 0) :
        std::numpunct_byname<char>(name,refs)
    {
    }
    virtual char do_thousands_sep() const
    {
        unsigned char bs = base_type::do_thousands_sep();
        if(bs > 127)
            if(bs == 0xA0)
                return ' ';
            else
                return 0;
        else
            return bs;
    }
    virtual std::string do_grouping() const
    {
        unsigned char bs = base_type::do_thousands_sep();
        if(bs > 127 && bs != 0xA0)
            return std::string();
        return base_type::do_grouping();
    }
};

template<bool Intl>
class utf8_moneypunct : public std::moneypunct_byname<char,Intl> {
public:
    typedef std::moneypunct_byname<char,Intl> base_type;
    utf8_moneypunct(char const *name,size_t refs = 0) :
        std::moneypunct_byname<char,Intl>(name,refs)
    {
    }
    virtual char do_thousands_sep() const
    {
        unsigned char bs = base_type::do_thousands_sep();
        if(bs > 127)
            if(bs == 0xA0)
                return ' ';
            else
                return 0;
        else
            return bs;
    }
    virtual std::string do_grouping() const
    {
        unsigned char bs = base_type::do_thousands_sep();
        if(bs > 127 && bs != 0xA0)
            return std::string();
        return base_type::do_grouping();
    }
};



#endif // BOOST_NO_STD_WSTRING

template<typename CharType>
std::locale create_basic_parsing(std::locale const &in,std::string const &locale_name)
{
    std::locale tmp = std::locale(in,new std::numpunct_byname<CharType>(locale_name.c_str()));
    tmp = std::locale(tmp,new std::moneypunct_byname<CharType,true>(locale_name.c_str()));
    tmp = std::locale(tmp,new std::moneypunct_byname<CharType,false>(locale_name.c_str()));
    tmp = std::locale(tmp,new std::ctype_byname<CharType>(locale_name.c_str()));
    return tmp;
}

template<typename CharType>
std::locale create_basic_formatting(std::locale const &in,std::string const &locale_name)
{
    std::locale tmp = create_basic_parsing<CharType>(in,locale_name);
    std::locale base(locale_name.c_str());
    tmp = std::locale(tmp,new time_put_from_base<CharType>(base));
    return tmp;
}


std::locale create_formatting(  std::locale const &in,
                                std::string const &locale_name,
                                character_facet_type type,
                                utf8_support utf)
{
        switch(type) {
        case char_facet: 
            {
                #ifndef BOOST_NO_STD_WSTRING
                if(utf == utf8_from_wide ) {
                    std::locale base = std::locale(locale_name.c_str());
                    
                    std::locale tmp = std::locale(in,new utf8_time_put_from_wide(base));
                    tmp = std::locale(tmp,new utf8_numpunct_from_wide(base));
                    tmp = std::locale(tmp,new utf8_moneypunct_from_wide<true>(base));
                    tmp = std::locale(tmp,new utf8_moneypunct_from_wide<false>(base));
                    return std::locale(tmp,new num_format<char>());
                }
                else if(utf == utf8_native) {
                    std::locale base = std::locale(locale_name.c_str());

                    std::locale tmp = std::locale(in,new time_put_from_base<char>(base));
                    tmp = std::locale(tmp,new utf8_numpunct(locale_name.c_str()));
                    tmp = std::locale(tmp,new utf8_moneypunct<true>(locale_name.c_str()));
                    tmp = std::locale(tmp,new utf8_moneypunct<false>(locale_name.c_str()));
                    return std::locale(tmp,new num_format<char>());
                }
                else if(utf == utf8_native_with_wide) {
                    std::locale base = std::locale(locale_name.c_str());

                    std::locale tmp = std::locale(in,new time_put_from_base<char>(base));
                    tmp = std::locale(tmp,new utf8_numpunct_from_wide(base));
                    tmp = std::locale(tmp,new utf8_moneypunct_from_wide<true>(base));
                    tmp = std::locale(tmp,new utf8_moneypunct_from_wide<false>(base));
                    return std::locale(tmp,new num_format<char>());
                }
                else
                #endif
                {
                    std::locale tmp = create_basic_formatting<char>(in,locale_name);
                    tmp = std::locale(tmp,new num_format<char>());
                    return tmp;
                }
            }
        #ifndef BOOST_NO_STD_WSTRING
        case wchar_t_facet:
            {
                std::locale tmp = create_basic_formatting<wchar_t>(in,locale_name);
                tmp = std::locale(tmp,new num_format<wchar_t>());
                return tmp;
            }
        #endif
        #ifdef BOOST_HAS_CHAR16_T
        case char16_t_facet:
            {
                std::locale tmp = create_basic_formatting<char16_t>(in,locale_name);
                tmp = std::locale(tmp,new num_format<char16_t>());
                return tmp;
            }
        #endif
        #ifdef BOOST_HAS_CHAR32_T
        case char32_t_facet:
            {
                std::locale tmp = create_basic_formatting<char32_t>(in,locale_name);
                tmp = std::locale(tmp,new num_format<char32_t>());
                return tmp
            }
        #endif
        default:
            return in;
        }
}

std::locale create_parsing( std::locale const &in,
                            std::string const &locale_name,
                            character_facet_type type,
                            utf8_support utf)
{
        switch(type) {
        case char_facet:
            {
                #ifndef BOOST_NO_STD_WSTRING
                if(utf == utf8_from_wide ) {
                    std::locale base = std::locale::classic();
                    
                    base = std::locale(base,new std::numpunct_byname<wchar_t>(locale_name.c_str()));
                    base = std::locale(base,new std::moneypunct_byname<wchar_t,true>(locale_name.c_str()));
                    base = std::locale(base,new std::moneypunct_byname<wchar_t,false>(locale_name.c_str()));

                    std::locale tmp = std::locale(in,new utf8_numpunct_from_wide(base));
                    tmp = std::locale(tmp,new utf8_moneypunct_from_wide<true>(base));
                    tmp = std::locale(tmp,new utf8_moneypunct_from_wide<false>(base));
                    return std::locale(tmp,new num_parse<char>());
                }
                else if(utf == utf8_native) {
                    std::locale tmp = std::locale(in,new utf8_numpunct(locale_name.c_str()));
                    tmp = std::locale(tmp,new utf8_moneypunct<true>(locale_name.c_str()));
                    tmp = std::locale(tmp,new utf8_moneypunct<false>(locale_name.c_str()));
                    return std::locale(tmp,new num_parse<char>());
                }
                else if(utf == utf8_native_with_wide) {
                    std::locale base = std::locale(locale_name.c_str());

                    std::locale tmp = std::locale(in,new utf8_numpunct_from_wide(base));
                    tmp = std::locale(tmp,new utf8_moneypunct_from_wide<true>(base));
                    tmp = std::locale(tmp,new utf8_moneypunct_from_wide<false>(base));
                    return std::locale(tmp,new num_parse<char>());
                }
                else 
                #endif
                {
                    std::locale tmp = create_basic_parsing<char>(in,locale_name);
                    tmp = std::locale(in,new num_parse<char>());
                    return tmp;
                }
            }
        #ifndef BOOST_NO_STD_WSTRING
        case wchar_t_facet:
                {
                    std::locale tmp = create_basic_parsing<wchar_t>(in,locale_name);
                    tmp = std::locale(in,new num_parse<wchar_t>());
                    return tmp;
                }
        #endif
        #ifdef BOOST_HAS_CHAR16_T
        case char16_t_facet:
                {
                    std::locale tmp = create_basic_parsing<char16_t>(in,locale_name);
                    tmp = std::locale(in,new num_parse<char16_t>());
                    return tmp;
                }
        #endif
        #ifdef BOOST_HAS_CHAR32_T
        case char32_t_facet:
                {
                    std::locale tmp = create_basic_parsing<char32_t>(in,locale_name);
                    tmp = std::locale(in,new num_parse<char32_t>());
                    return tmp;
                }
        #endif
        default:
            return in;
        }
}


} // impl_std
} // locale 
} //boost



// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
