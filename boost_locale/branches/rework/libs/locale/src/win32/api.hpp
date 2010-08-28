//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_IMPL_WIN32_API_HPP
#define BOOST_LOCALE_IMPL_WIN32_API_HPP

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <time.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>

#define BOOST_LOCALE_WINDOWS_2000_API

#if defined(_WIN32_NT) && _WIN32_NT >= 0x600 && !defined(BOOST_LOCALE_WINDOWS_2000_API)
#define BOOST_LOCALE_WINDOWS_VISTA_API
#else
#define BOOST_LOCALE_WINDOWS_2000_API
#endif

namespace boost {
namespace locale {
namespace impl_win {
    
    class numeric_info {
        std::wstring thousands_sep;
        std::wstring decimal_point;
        std::string grouping;
    };
  
    #ifdef BOOST_LOCALE_WINDOWS_2000_API 
    
    class winlocale{
    public:
        unsigned lcid;
    };

    ////////////////////////////////////////////////////////////////////////
    ///
    /// Collation
    ///
    ////////////////////////////////////////////////////////////////////////

    inline int wcscoll_l(wchar_t const *lb,wchar_t const *le,wchar_t const *rb,wchar_t const *re,winlocale const &l)
    {
        return CompareStringW(l.lcid,0,lb,le-lb,rb,re-rb) - 2;
    }

    inline std::wstring wcsxfrm_l(wchar_t const *begin,wchar_t const *end,winlocale const &l)
    {
        std::wstring res;
        int len = LCMapStringW(l.lcid,0,begin,end-begin,0,0);
        if(len == 0)
            return res;
        std::vector<wchar_t> buf(len);
        LCMapStringW(l.lcid,0,begin,end-begin,&buf.front(),len);
        res.assign(&buf.front(),len-1);
        return res;
    }

    ////////////////////////////////////////////////////////////////////////
    ///
    /// Money Format
    ///
    ////////////////////////////////////////////////////////////////////////

    inline std::wstring wcsfmon_l(double value,winlocale const &l)
    {
        std::wostringstream ss;
        ss.imbue(std::locale::classic());
        ss << std::setprecision(std::numeric_limits<double>::digits10+1) << value;
        std::wstring sval = ss.str();
        int len = GetCurrencyFormatW(l.lcid,0,sval.c_str(),0,0,0);
        std::vector<wchar_t> buf(len+1);
        GetCurrencyFormatW(l.lcid,0,sval.c_str(),0,&buf.front(),len);
        return &buf.front();
    }

    ////////////////////////////////////////////////////////////////////////
    ///
    /// Time Format
    ///
    ////////////////////////////////////////////////////////////////////////

    
    inline std::wstring wcs_format_date_l(wchar_t const *format,SYSTEMTIME const *tm,winlocale const &l)
    {
        int len = GetDateFormatW(l.lcid,0,tm,format,0,0);
        std::vector<wchar_t> buf(len+1);
        GetDateFormatW(l.lcid,0,tm,format,&buf.front(),len);
        return &buf.front(); 
    }
    
    inline std::wstring wcs_format_time_l(wchar_t const *format,SYSTEMTIME const *tm,winlocale const &l)
    {
        int len = GetTimeFormatW(l.lcid,0,tm,format,0,0);
        std::vector<wchar_t> buf(len+1);
        GetTimeFormatW(l.lcid,0,tm,format,&buf.front(),len);
        return &buf.front(); 
    }


    #endif

    inline std::wstring wcsftime_l(char c,std::tm const *tm,winlocale const &l)
    {
        SYSTEMTIME wtm=SYSTEMTIME();
        wtm.wYear = tm->tm_year + 1900;
        wtm.wMonth = tm->tm_mon+1;
        wtm.wDayOfWeek = tm->tm_wday;
        wtm.wDay = tm->tm_mday;
        wtm.wHour = tm->tm_hour;
        wtm.wMinute = tm->tm_min;
        wtm.wSecond = tm->tm_sec;
        switch(c) {
        case 'a': // Abbr Weekday
            return wcs_format_date_l(L"ddd",&wtm,l);
        case 'A': // Full Weekday
            return wcs_format_date_l(L"dddd",&wtm,l);
        case 'b': // Abbr Month
            return wcs_format_date_l(L"MMM",&wtm,l);
        case 'B': // Full Month
            return wcs_format_date_l(L"MMMM",&wtm,l);
        case 'c': // DateTile Full
            return wcs_format_date_l(0,&wtm,l) + L" " + wcs_format_time_l(0,&wtm,l);
        // not supported by WIN ;(
        //  case 'C': // Century -> 1980 -> 19
        //  retur
        case 'd': // Day of Month [01,31]
            return wcs_format_date_l(L"dd",&wtm,l);
        case 'D': // %m/%d/%y
            return wcs_format_date_l(L"MM/dd/yy",&wtm,l);
        case 'e': // Day of Month [1,31]
            return wcs_format_date_l(L"d",&wtm,l);
        case 'h': // == b
            return wcs_format_date_l(L"MMM",&wtm,l);
        case 'H': // 24 clock hour 00,23
            return wcs_format_time_l(L"HH",&wtm,l);
        case 'I': // 12 clock hour 01,12
            return wcs_format_time_l(L"hh",&wtm,l);
        /*
        case 'j': // day of year 001,366
            return "D";*/
        case 'm': // month as [01,12]
            return wcs_format_date_l(L"MM",&wtm,l);
        case 'M': // minute [00,59]
            return wcs_format_time_l(L"mm",&wtm,l);
        case 'n': // \n
            return L"\n";
        case 'p': // am-pm
            return wcs_format_time_l(L"tt",&wtm,l);
        case 'r': // time with AM/PM %I:%M:%S %p
            return wcs_format_time_l(L"hh:mm:ss tt",&wtm,l);
        case 'R': // %H:%M
            return wcs_format_time_l(L"HH:mm",&wtm,l);
        case 'S': // second [00,61]
            return wcs_format_time_l(L"ss",&wtm,l);
        case 't': // \t
            return L"\t";
        case 'T': // %H:%M:%S
            return wcs_format_time_l(L"HH:mm:ss",&wtm,l);
/*          case 'u': // weekday 1,7 1=Monday
        case 'U': // week number of year [00,53] Sunday first
        case 'V': // week number of year [01,53] Moday first
        case 'w': // weekday 0,7 0=Sunday
        case 'W': // week number of year [00,53] Moday first, */
        case 'x': // Date
            return wcs_format_date_l(0,&wtm,l);
        case 'X': // Time
            return wcs_format_time_l(0,&wtm,l);
        case 'y': // Year [00-99]
            return wcs_format_date_l(L"yy",&wtm,l);
        case 'Y': // Year 1998
            return wcs_format_date_l(L"yyyy",&wtm,l);
        case '%': // %
            return L"%";
        default:
            return L"";
        }
    }



} // win
} // locale
} // boost
#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
