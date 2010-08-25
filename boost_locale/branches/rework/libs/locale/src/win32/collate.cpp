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
#include <boost/locale/codepage.hpp>
#include "all_generator.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace boost {
namespace locale {
namespace impl_win {

namespace {

    int wcscoll_lcid(int lcid,wchar_t const *lb,wchar_t const *le,wchar_t const *rb,wchar_t const *re)
    {
        return CompareStringW(lcid,0,lb,le-lb,rb,re-rb) - 2;
    }

    std::wstring wcsxfrm_lcid(int lcid,wchar_t const *begin,wchar_t const *end)
    {
        std::wstring res;
        int len = LCMapStringW(lcid,0,begin,end-begin,0,0);
        if(len == 0)
            return res;
        std::vector<wchar_t> buf(len);
        LCMapStringW(lcid,0,begin,end-begin,&buf.front(),len);
        res.assign(&buf.front(),len-1);
        return res;
    }

}

class utf8_collator_from_wide : public std::collate<char> {
public:
    typedef std::collate<wchar_t> wfacet;
    utf8_collator_from_wide(int lcid,size_t refs = 0) : 
        std::collate<char>(refs),
        lcid_(lcid)
    {
    }
    virtual int do_compare(char const *lb,char const *le,char const *rb,char const *re) const
    {
        std::wstring l=conv::to_utf<wchar_t>(lb,le,"UTF-8");
        std::wstring r=conv::to_utf<wchar_t>(rb,re,"UTF-8");
        return wcscoll_lcid(lcid_,l.c_str(),l.c_str()+l.size(),r.c_str(),r.c_str()+r.size());
    }
    virtual long do_hash(char const *b,char const *e) const
    {
        return gnu_gettext::pj_winberger_hash_function(do_transform(b,e));
    }
    virtual std::string do_transform(char const *b,char const *e) const
    {
        std::wstring tmp=conv::to_utf<wchar_t>(b,e,"UTF-8");
        std::wstring wkey = wcsxfrm_lcid(lcid_,tmp.c_str(),tmp.c_str()+tmp.size());
        std::string key;
        if(sizeof(wchar_t)==2)
            key.reserve(wkey.size()*2);
        else
            key.reserve(wkey.size()*3);
        for(unsigned i=0;i<wkey.size();i++) {
            if(sizeof(wchar_t)==2) {
                uint16_t tv = static_cast<uint16_t>(wkey[i]);
                key += char(tv >> 8);
                key += char(tv & 0xFF);
            }
            else { // 4
                uint32_t tv = static_cast<uint32_t>(wkey[i]);
                // 21 bit
                key += char((tv >> 16) & 0xFF);
                key += char((tv >> 8) & 0xFF);
                key += char(tv & 0xFF);
            }
        }
        return key;
    }
private:
    std::locale base_;
};


class utf16_collator : public std::collate<wchar_t> {
public:
    typedef std::collate<wchar_t> wfacet;
    utf16_collator(int lcid,size_t refs = 0) : 
        std::collate<char>(refs),
        lcid_(lcid)
    {
    }
    virtual int do_compare(wchar_t const *lb,wchar_t const *le,wchar_t const *rb,wchar_t const *re) const
    {
        return wcscoll_lcid(lcid_,lb,le,rb,re);
    }
    virtual long do_hash(wchar_t const *b,wchar_t const *e) const
    {
        return gnu_gettext::pj_winberger_hash_function(reinterpret_cast<char const *>(do_transform(b,e).c_str()));
    }
    virtual std::wstring do_transform(char const *b,char const *e) const
    {
        return wcsxfrm_lcid(lcid_,b,e);
    }
private:
    int lcid_;
};


std::locale create_collate( std::locale const &in,
                            int lcid,
                            character_facet_type type)
{
    switch(type) {
    case char_facet:
        return std::locale(in,new utf8_collator_from_wide(lcid));
    case wchar_t_facet:
        return std::locale(in,new utf16_collator(lcid));
    default:
        return in;
    }
}


} // impl_std
} // locale 
} //boost



// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
