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

#if defined(__linux) || defined(__APPLE__)
#define BOOST_LOCALE_HAVE_WCSFTIME_L
#endif

namespace boost {
namespace locale {
namespace impl_posix {

template<typename CharType>
class num_format : public std::num_put<CharType>
{
public:
    typedef typename std::num_put<CharType>::iter_type iter_type;
    typedef std::basic_string<CharType> string_type;
    typedef CharType char_type;

    num_format(boost::shared_ptr<locale_t> lc,size_t refs = 0) : 
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

    iter_type format_currency(bool intl,iter_type out,std::ios_base &ios,char_type fill,long double val) const
    {
        char buf[64];
        char const *format = intl ? "%i" : "%n";
        size_t n = strfmon_l(buf,256,*lc_,format,static_cast<double>(val));
        std::string formatted;
        if(n==0) {
            std::vector<char> tmp(1024);
            n = strfmon_l(&tmp.front(),1024,*lc_,format,static_cast<double>(val));
            return write_it(out,&tmp.front(),n);
        }
        else
            return write_it(out,buf,n);
    }

    std::ostreambuf_iterator<char> write_it(std::ostreambuf_iterator<char> out,char const *ptr,size_t n) const
    {
        for(size_t i=0;i<n;i++)
            *out++ = *ptr++;
        return out;
    }
    
    std::ostreambuf_iterator<wchar_t> write_it(std::ostreambuf_iterator<wchar_t> out,char const *ptr,size_t n) const
    {
        std::wstring tmp = conv::to_utf<wchar_t>(ptr,ptr+n,nl_langinfo_l(CODESET,*lc_));
        for(size_t i=0;i<tmp.size();i++)
            *out++ = tmp[i];
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
private:
    boost::shared_ptr<locale_t> lc_;

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
struct ftime_traits;

template<>
struct ftime_traits<char> {
    static std::string ftime(char const *format,const struct tm *t,locale_t lc)
    {
        char buf[16];
        size_t n=strftime_l(buf,sizeof(buf),format,t,lc);
        if(n == 0) {
            // should be big enough
            std::vector<char> v(1024);
            n = strftime_l(&v.front(),1024,format,t,lc);
            return std::string(&v.front(),n);
        }
        return std::string(buf,n);
    }
};

template<>
struct ftime_traits<wchar_t> {
    static std::wstring ftime(wchar_t const *format,const struct tm *t,locale_t lc)
    {
        #ifdef HAVE_WCSFTIME_L
            wchar_t buf[16];
            size_t n=wcsftime_l(buf,16,format,t,lc);
            if(n == 0) {
                // should be big enough
                std::vector<wchar_t> v(1024);
                n = wcsftime_l(&v.front(),1024,format,t,lc);
            }
            return std::wstring(&v.front(),n);
        #else
            std::string enc = nl_langinfo_l(CODESET,lc);
            std::string nformat = conv::from_utf<wchar_t>(format,enc);
            std::string nres = ftime_traits<char>::ftime(nformat.c_str(),t,lc);
            return conv::to_utf<wchar_t>(nres,enc);
        #endif
    }
};


template<typename CharType>
class time_put_posix : public std::time_put<CharType> {
public:
    time_put_posix(boost::shared_ptr<locale_t> lc, size_t refs = 0) : 
        std::time_put<CharType>(refs),
        lc_(lc)
    {
    }
    virtual ~time_put_posix()
    {
    }
    typedef typename std::time_put<CharType>::iter_type iter_type;
    typedef CharType char_type;
    typedef std::basic_string<char_type> string_type;

    virtual iter_type do_put(iter_type out,std::ios_base &ios,CharType fill,std::tm const *tm,char format,char modifier) const
    {
        char_type fmt[4] = { '%' , modifier != 0 ? modifier : format , modifier == 0 ? 0 : format };
        string_type res = ftime_traits<char_type>::ftime(fmt,tm,*lc_);
        for(unsigned i=0;i<res.size();i++)
            *out++ = res[i];
        return out;
    }

private:
    boost::shared_ptr<locale_t> lc_;
};


template<typename CharType>
class ctype_posix;

template<>
class ctype_posix<char> : public std::ctype<char> {
public:

    ctype_posix(boost::shared_ptr<locale_t> lc) 
    {
        lc_ = lc;
    }
   
    bool do_is(mask m,char c) const
    {
        if((m & space) && isspace_l(c,*lc_))
            return true;
        if((m & print) && isprint_l(c,*lc_))
            return true;
        if((m & cntrl) && iscntrl_l(c,*lc_))
            return true;
        if((m & upper) && isupper_l(c,*lc_))
            return true;
        if((m & lower) && islower_l(c,*lc_))
            return true;
        if((m & alpha) && isalpha_l(c,*lc_))
            return true;
        if((m & digit) && isdigit_l(c,*lc_))
            return true;
        if((m & xdigit) && isxdigit_l(c,*lc_))
            return true;
        if((m & punct) && ispunct_l(c,*lc_))
            return true;
        return false;
    }
    char const *do_is(char const *begin,char const *end,mask *m) const
    {
        while(begin!=end) {
            char c= *begin++;
            mask r=0;
            if(isspace_l(c,*lc_))
                r|=space;
            if(isprint_l(c,*lc_))
                r|=cntrl;
            if(iscntrl_l(c,*lc_))
                r|=space;
            if(isupper_l(c,*lc_))
                r|=upper;
            if(islower_l(c,*lc_))
                r|=lower;
            if(isalpha_l(c,*lc_))
                r|=alpha;
            if(isdigit_l(c,*lc_))
                r|=digit;
            if(isxdigit_l(c,*lc_))
                r|=xdigit;
            if(ispunct_l(c,*lc_))
                r|=punct;
            *m++ = r;
        }
        return begin;
    }
    char const *do_scan_is(mask m,char const *begin,char const *end) const
    {
        while(begin!=end)
            if(do_is(m,*begin))
                return begin;
        return begin;
    }
    char const *do_scan_not(mask m,char const *begin,char const *end) const
    {
        while(begin!=end)
            if(!do_is(m,*begin))
                return begin;
        return begin;
    }
    char toupper(char c) const
    {
        return toupper_l(c,*lc_);
    }
    char const *toupper(char *begin,char const *end) const
    {
        for(;begin!=end;begin++)
            *begin = toupper_l(*begin,*lc_);
        return begin;
    }
    char tolower(char c) const
    {
        return tolower_l(c,*lc_);
    }
    char const *tolower(char *begin,char const *end) const
    {
        for(;begin!=end;begin++)
            *begin = tolower_l(*begin,*lc_);
        return begin;
    }
private:
    boost::shared_ptr<locale_t> lc_;
};

template<>
class ctype_posix<wchar_t> : public std::ctype<wchar_t> {
public:
    ctype_posix(boost::shared_ptr<locale_t> lc) 
    {
        lc_ = lc;
    }
   
    bool do_is(mask m,wchar_t c) const
    {
        if((m & space) && iswspace_l(c,*lc_))
            return true;
        if((m & print) && iswprint_l(c,*lc_))
            return true;
        if((m & cntrl) && iswcntrl_l(c,*lc_))
            return true;
        if((m & upper) && iswupper_l(c,*lc_))
            return true;
        if((m & lower) && iswlower_l(c,*lc_))
            return true;
        if((m & alpha) && iswalpha_l(c,*lc_))
            return true;
        if((m & digit) && iswdigit_l(c,*lc_))
            return true;
        if((m & xdigit) && iswxdigit_l(c,*lc_))
            return true;
        if((m & punct) && iswpunct_l(c,*lc_))
            return true;
        return false;
    }
    wchar_t const *do_is(wchar_t const *begin,wchar_t const *end,mask *m) const
    {
        while(begin!=end) {
            wchar_t c= *begin++;
            mask r=0;
            if(iswspace_l(c,*lc_))
                r|=space;
            if(iswprint_l(c,*lc_))
                r|=cntrl;
            if(iswcntrl_l(c,*lc_))
                r|=space;
            if(iswupper_l(c,*lc_))
                r|=upper;
            if(iswlower_l(c,*lc_))
                r|=lower;
            if(iswalpha_l(c,*lc_))
                r|=alpha;
            if(iswdigit_l(c,*lc_))
                r|=digit;
            if(iswxdigit_l(c,*lc_))
                r|=xdigit;
            if(iswpunct_l(c,*lc_))
                r|=punct;
            *m++ = r;
        }
        return begin;
    }
    wchar_t const *do_scan_is(mask m,wchar_t const *begin,wchar_t const *end) const
    {
        while(begin!=end)
            if(do_is(m,*begin))
                return begin;
        return begin;
    }
    wchar_t const *do_scan_not(mask m,wchar_t const *begin,wchar_t const *end) const
    {
        while(begin!=end)
            if(!do_is(m,*begin))
                return begin;
        return begin;
    }
    wchar_t toupper(wchar_t c) const
    {
        return towupper_l(c,*lc_);
    }
    wchar_t const *toupper(wchar_t *begin,wchar_t const *end) const
    {
        for(;begin!=end;begin++)
            *begin = towupper_l(*begin,*lc_);
        return begin;
    }
    wchar_t tolower(wchar_t c) const
    {
        return tolower_l(c,*lc_);
    }
    wchar_t const *tolower(wchar_t *begin,wchar_t const *end) const
    {
        for(;begin!=end;begin++)
            *begin = tolower_l(*begin,*lc_);
        return begin;
    }
private:
    boost::shared_ptr<locale_t> lc_;
};




struct basic_numpunct {
    std::string grouping;
    std::string thousands_sep;
    std::string decimal_point;
    basic_numpunct() : 
        decimal_point(".")
    {
    }
    basic_numpunct(locale_t lc) 
    {
    #ifdef __APPLE__
        lconv *cv = localeconv_l(lc);
        grouping = cv->grouping;
        thousands_sep = cv->thousands_sep;
        decimal_point = cv->decimal_point;
    #else
        thousands_sep = nl_langinfo_l(THOUSEP,lc);
        decimal_point = nl_langinfo_l(RADIXCHAR,lc);
        #ifdef GROUPING
        grouping = nl_langinfo_l(GROUPING,lc);
        #endif
    #endif
    }
};

template<typename CharType>
class num_punct_posix : public std::numpunct<CharType> {
public:
    typedef std::basic_string<CharType> string_type;
    num_punct_posix(locale_t lc,size_t refs = 0) : 
        std::numpunct<CharType>(refs)
    {
        basic_numpunct np(lc);
        to_str(np.thousands_sep,thousands_sep_,lc);
        to_str(np.decimal_point,decimal_point_,lc);
        grouping_ = np.grouping;
        if(thousands_sep_.size() > 1)
            grouping_ = std::string();
        if(decimal_point_.size() > 1)
            decimal_point_ = CharType('.');
    }
    void to_str(std::string &s1,std::string &s2,locale_t lc)
    {
        s2.swap(s1);
    }
    void to_str(std::string &s1,std::wstring &s2,locale_t lc)
    {
        s2=conv::to_utf<wchar_t>(s1,nl_langinfo_l(CODESET,lc));
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
std::locale create_formatting_impl(std::locale const &in,boost::shared_ptr<locale_t> lc)
{
    std::locale tmp = std::locale(in,new num_punct_posix<CharType>(*lc));
    tmp = std::locale(tmp,new ctype_posix<CharType>(lc));
    tmp = std::locale(tmp,new time_put_posix<CharType>(lc));
    tmp = std::locale(tmp,new num_format<CharType>(lc));
    return tmp;
}

template<typename CharType>
std::locale create_parsing_impl(std::locale const &in,boost::shared_ptr<locale_t> lc)
{
    std::locale tmp = std::locale(in,new num_punct_posix<CharType>(*lc));
    tmp = std::locale(tmp,new ctype_posix<CharType>(lc));
    tmp = std::locale(tmp,new num_parse<CharType>());
    return tmp;
}


std::locale create_formatting(  std::locale const &in,
                                boost::shared_ptr<locale_t> lc,
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
                            boost::shared_ptr<locale_t> lc,
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
