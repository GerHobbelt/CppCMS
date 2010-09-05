//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_LOCALE_SOURCE
#include <boost/locale/generator.hpp>
#include <boost/locale/encoding.hpp>

#include "../encoding/conv.hpp"

#include "codecvt_converter.hpp"
#ifdef BOOST_MSVC
#  pragma warning(disable : 4244) // loose data 
#endif


#include <string.h>
#include <vector>
#include <algorithm>

namespace boost {
namespace locale {
namespace util {
    
    class utf8_converter  : public base_converter {
    public:
        virtual int max_len() const
        {
            return 4;
        }

        virtual utf8_converter *clone() const
        {
            return new utf8_converter();
        }

        bool is_thread_safe() const
        {
            return true;
        }

        virtual uint32_t to_unicode(char const *&begin,char const *end)
        {
            unsigned char const *p=reinterpret_cast<unsigned char const *>(begin);
            unsigned char const *e=reinterpret_cast<unsigned char const *>(end);
            if(p==e)
                return incomplete;
            unsigned char c=*p++;
            unsigned char seq0,seq1=0,seq2=0,seq3=0;
            seq0=c;
            int len=1;
            if((c & 0xC0) == 0xC0) {
                if(p==e)
                    return incomplete;
                seq1=*p++;
                len=2;
            }
            if((c & 0xE0) == 0xE0) {
                if(p==e)
                    return incomplete;
                seq2=*p++;
                len=3;
            }
            if((c & 0xF0) == 0xF0) {
                if(p==e)
                    return incomplete;
                seq3=*p++;
                len=4;
            }
            switch(len) {
            case 1:
                break;
            case 2: // non-overloading 2 bytes
                if( 0xC2 <= seq0 && seq0 <= 0xDF
                    && 0x80 <= seq1 && seq1<= 0xBF)
                {
                        break;
                }
                return illegal;
            case 3: 
                if(seq0==0xE0) { // exclude overloading
                    if(0xA0 <=seq1 && seq1<= 0xBF && 0x80 <=seq2 && seq2<=0xBF)
                        break;
                }
                else if( (0xE1 <= seq0 && seq0 <=0xEC) || seq0==0xEE || seq0==0xEF) { // stright 3 bytes
                    if(0x80 <=seq1 && seq1<=0xBF &&
                       0x80 <=seq2 && seq2<=0xBF)
                        break;
                }
                else if(seq0 == 0xED) { // exclude surrogates
                    if( 0x80 <=seq1 && seq1<=0x9F &&
                        0x80 <=seq2 && seq2<=0xBF)
                        break;
                }
                return illegal;
            case 4:
                switch(seq0) {
                case 0xF0: // planes 1-3
                    if( 0x90 <=seq1 && seq1<=0xBF &&
                        0x80 <=seq2 && seq2<=0xBF &&
                        0x80 <=seq3 && seq3<=0xBF)
                        break;
                    return illegal;
                case 0xF1: // planes 4-15
                case 0xF2:
                case 0xF3:
                    if( 0x80 <=seq1 && seq1<=0xBF &&
                        0x80 <=seq2 && seq2<=0xBF &&
                        0x80 <=seq3 && seq3<=0xBF)
                        break;
                    return illegal;
                case 0xF4: // pane 16
                    if( 0x80 <=seq1 && seq1<=0x8F &&
                        0x80 <=seq2 && seq2<=0xBF &&
                        0x80 <=seq3 && seq3<=0xBF)
                        break;
                    return illegal;
                default:
                    return illegal;
                }
            }
            begin=reinterpret_cast<char const *>(p);
            switch(len) {
            case 1:
                return seq0;
            case 2:
                return ((seq0 & 0x1F) << 6) | (seq1 & 0x3F);
            case 3:
                return ((seq0 & 0x0F) << 12) | ((seq1 & 0x3F) << 6) | (seq2 & 0x3F)  ;
            default: // can be only 4
                return ((seq0 & 0x07) << 18) | ((seq1 & 0x3F) << 12) | ((seq2 & 0x3F) << 6) | (seq3 & 0x3F) ;
            }
        }
        virtual uint32_t from_unicode(uint32_t u,char *begin,char const *end) 
        {
            if(u>0x10ffff)
                return illegal;
            if(0xd800 <=u && u<= 0xdfff) // surrogates
                return illegal;
            ptrdiff_t d=end-begin;
            if(u <=0x7F) { 
                if(d>=1) {
                    *begin++=u;
                    return 1;
                }
                else
                    return incomplete;
            }
            else if(u <= 0x7FF) {
                if(d>=2) {
                    *begin++=(u >> 6) | 0xC0;
                    *begin++=(u & 0x3F) | 0x80;
                    return 2;
                }
                else
                    return incomplete;
            }
            else if(u <= 0xFFFF) {
                if(d>=3) {
                    *begin++=(u >> 12) | 0xE0;
                    *begin++=((u >> 6) & 0x3F) | 0x80;
                    *begin++=(u & 0x3F) | 0x80;
                    return 3;
                }
                else
                    return incomplete;
            }
            else {
                if(d>=4) {
                    *begin++=(u >> 18) | 0xF0;
                    *begin++=((u >> 12) & 0x3F) | 0x80;
                    *begin++=((u >> 6) & 0x3F) | 0x80;
                    *begin++=(u & 0x3F) | 0x80;
                    return 4;
                }
                else
                    return incomplete;
            }
        }
    }; // utf8_converter

    class simple_converter : public base_converter {
    public:

        virtual ~simple_converter() 
        {
        }

        simple_converter(std::string const &encoding)
        {
            for(unsigned i=0;i<128;i++)
                to_unicode_tbl_[i]=i;
            for(unsigned i=128;i<256;i++) {
                char buf[2] = { i , 0 };
                std::wstring const tmp = conv::to_utf<wchar_t>(buf,buf+1,encoding);
                if(tmp.size() != 1) {
                    to_unicode_tbl_[i] = illegal;
                }
                else {
                    to_unicode_tbl_[i] = tmp[0];
                }
            }
            from_unicode_tbl_.resize(256);
            for(unsigned i=0;i<256;i++) {
                from_unicode_tbl_[to_unicode_tbl_[i] & 0xFF].push_back(i);
            }
        }

        virtual int max_len() const 
        {
            return 1;
        }

        virtual bool is_thread_safe() const 
        {
            return true;
        }
        virtual base_converter *clone() const 
        {
           return new simple_converter(*this); 
        }
        virtual uint32_t to_unicode(char const *&begin,char const *end)
        {
            if(begin==end)
                return incomplete;
            unsigned char c = *begin++;
            return to_unicode_tbl_[c];
        }
        virtual uint32_t from_unicode(uint32_t u,char *begin,char const *end)
        {
            if(begin==end)
                return incomplete;
            std::vector<unsigned char> const &tbl = from_unicode_tbl_[u & 0xFF];
            for(std::vector<unsigned char>::const_iterator p=tbl.begin();p!=tbl.end();++p) {
                if(to_unicode_tbl_[*p]==u) {
                    *begin++ = *p;
                    return 1;
                }
            }
            return illegal;
        }
    private:
        uint32_t to_unicode_tbl_[256];
        std::vector<std::vector<unsigned char> > from_unicode_tbl_;
    };

    namespace {
        char const *simple_encoding_table[] = {
            "cp1250",
            "cp1251",
            "cp1252",
            "cp1253",
            "cp1254",
            "cp1255",
            "cp1256",
            "cp1257",
            "iso88591",
            "iso885913",
            "iso885915",
            "iso88592",
            "iso88593",
            "iso88594",
            "iso88595",
            "iso88596",
            "iso88597",
            "iso88598",
            "iso88599",
            "koi8r",
            "koi8u",
            "windows1250",
            "windows1251",
            "windows1252",
            "windows1253",
            "windows1254",
            "windows1255",
            "windows1256",
            "windows1257"
        };

        bool compare_strings(char const *l,char const *r)
        {
            return strcmp(l,r) < 0;
        }
    }

    
    std::auto_ptr<base_converter> create_simple_converter(std::string const &encoding)
    {
        std::auto_ptr<base_converter> res;
        std::string norm = conv::impl::normalize_encoding(encoding.c_str());
       if(std::binary_search<char const **>( simple_encoding_table,
                        simple_encoding_table + sizeof(simple_encoding_table)/sizeof(char const *),
                        norm.c_str(),
                        compare_strings))
        {
            res.reset(new simple_converter(encoding));
        }
        return res;
    }



    std::auto_ptr<base_converter> create_utf8_converter()
    {
        std::auto_ptr<base_converter> res(new utf8_converter());
        return res;
    }
    
    //
    // Traits for sizeof char
    //
    template<typename CharType,int n=sizeof(CharType)>
    struct uchar_traits;

    template<typename CharType>
    struct uchar_traits<CharType,2> {
        typedef uint16_t uint_type;
    };
    template<typename CharType>
    struct uchar_traits<CharType,4> {
        typedef uint32_t uint_type;
    };

    // Real codecvt

    template<typename CharType>
    class code_converter : public std::codecvt<CharType,char,mbstate_t> 
    {
    public:
        code_converter(std::auto_ptr<base_converter> cvt,size_t refs = 0) :
            std::codecvt<CharType,char,mbstate_t>(refs),
            cvt_(cvt)
        {
            max_len_ = cvt_->max_len(); 
        }
    protected:

        typedef CharType uchar;

        virtual std::codecvt_base::result do_unshift(std::mbstate_t &s,char *from,char *to,char *&next) const
        {
            next=from;
            return std::codecvt_base::ok;
        }
        virtual int do_encoding() const throw()
        {
            return 0;
        }
        virtual int do_max_length() const throw()
        {
            return max_len_;
        }
        virtual bool do_always_noconv() const throw()
        {
            return false;
        }
        
        virtual std::codecvt_base::result 
        do_in(  std::mbstate_t &state,
                char const *from,
                char const *from_end,
                char const *&from_next,
                uchar *uto,
                uchar *uto_end,
                uchar *&uto_next) const
        {
            typedef typename uchar_traits<uchar>::uint_type uint_type;
            uint_type *to=reinterpret_cast<uint_type *>(uto);
            uint_type *to_end=reinterpret_cast<uint_type *>(uto_end);
            uint_type *&to_next=reinterpret_cast<uint_type *&>(uto_next);
            return do_real_in(state,from,from_end,from_next,to,to_end,to_next);
        }
        
        virtual int
        do_length(  std::mbstate_t &state,
                char const *from,
                char const *from_end,
                size_t max) const
        {
            char const *from_next=from;
            std::vector<uchar> chrs(max+1);
            uchar *to=&chrs.front();
            uchar *to_end=to+max;
            uchar *to_next=to;
            do_in(state,from,from_end,from_next,to,to_end,to_next);
            return from_next-from;
        }

        virtual std::codecvt_base::result 
        do_out( std::mbstate_t &state,
                uchar const *ufrom,
                uchar const *ufrom_end,
                uchar const *&ufrom_next,
                char *to,
                char *to_end,
                char *&to_next) const
        {
            typedef typename uchar_traits<uchar>::uint_type uint_type;
            uint_type const *from=reinterpret_cast<uint_type const *>(ufrom);
            uint_type const *from_end=reinterpret_cast<uint_type const *>(ufrom_end);
            uint_type const *&from_next=reinterpret_cast<uint_type const *&>(ufrom_next);
            return do_real_out(state,from,from_end,from_next,to,to_end,to_next);
        }
       

    private:

        //
        // Implementation for UTF-32
        //
        std::codecvt_base::result
        do_real_in( std::mbstate_t &state,
                    char const *from,
                    char const *from_end,
                    char const *&from_next,
                    uint32_t *to,
                    uint32_t *to_end,
                    uint32_t *&to_next) const
        {
            std::auto_ptr<base_converter> cvtp;
            base_converter *cvt = 0;
            if(cvt_->is_thread_safe()) {
                cvt = cvt_.get();
            }
            else {
                cvtp.reset(cvt_->clone());
                cvt = cvtp.get();
            }
            std::codecvt_base::result r=std::codecvt_base::ok;
            while(to < to_end && from < from_end)
            {
                uint32_t ch=cvt->to_unicode(from,from_end);
                if(ch==base_converter::illegal) {
                    r=std::codecvt_base::error;
                    break;
                }
                if(ch==base_converter::incomplete) {
                    r=std::codecvt_base::partial;
                    break;
                }
                *to++=ch;
            }
            from_next=from;
            to_next=to;
            if(r!=std::codecvt_base::ok)
                return r;
            if(from!=from_end)
                return std::codecvt_base::partial;
            return r;
        }

        //
        // Implementation for UTF-32
        //
        std::codecvt_base::result
        do_real_out(std::mbstate_t &state,
                    uint32_t const *from,
                    uint32_t const *from_end,
                    uint32_t const *&from_next,
                    char *to,
                    char *to_end,
                    char *&to_next) const
        {
            std::auto_ptr<base_converter> cvtp;
            base_converter *cvt = 0;
            if(cvt_->is_thread_safe()) {
                cvt = cvt_.get();
            }
            else {
                cvtp.reset(cvt_->clone());
                cvt = cvtp.get();
            }
            
            std::codecvt_base::result r=std::codecvt_base::ok;
            while(to < to_end && from < from_end)
            {
                uint32_t len=cvt->from_unicode(*from,to,to_end);
                if(len==base_converter::illegal) {
                    r=std::codecvt_base::error;
                    break;
                }
                if(len==base_converter::incomplete) {
                    r=std::codecvt_base::partial;
                    break;
                }
                from++;
                to+=len;
            }
            from_next=from;
            to_next=to;
            if(r!=std::codecvt_base::ok)
                return r;
            if(from!=from_end)
                return std::codecvt_base::partial;
            return r;
        }

        //
        // Can't handle full UTF-16, only UCS-2
        // because:
        //   1. codecvt facet must be able to work on single
        //      internal character ie if  do_in(s,from,from_end,x,y,z,t) returns ok
        //      then do_in(s,from,from+1) should return ok according to the standard papars
        //   2. I have absolutly NO information about mbstat_t -- I can't even know if its 0 
        //      or it is somehow initialized. So I can't store any state information
        //      about suragate pairs... So it works only for UCS-2
        //
        
        
        //
        // Implementation for UTF-16
        //
        std::codecvt_base::result
        do_real_in( std::mbstate_t &state,
                    char const *from,
                    char const *from_end,
                    char const *&from_next,
                    uint16_t *to,
                    uint16_t *to_end,
                    uint16_t *&to_next) const
        {
            std::auto_ptr<base_converter> cvtp;
            base_converter *cvt = 0;
            if(cvt_->is_thread_safe()) {
                cvt = cvt_.get();
            }
            else {
                cvtp.reset(cvt_->clone());
                cvt = cvtp.get();
            }
            std::codecvt_base::result r=std::codecvt_base::ok;
            while(to < to_end && from < from_end)
            {
                uint32_t ch=cvt->to_unicode(from,from_end);
                if(ch==base_converter::illegal) {
                    r=std::codecvt_base::error;
                    break;
                }
                if(ch==base_converter::incomplete) {
                    r=std::codecvt_base::partial;
                    break;
                }
                if(ch <= 0xFFFF) {
                    *to++=ch;
                }
                else { /// can't handle surrogates
                    r=std::codecvt_base::error;
                    break;
                }
            }
            from_next=from;
            to_next=to;
            if(r!=std::codecvt_base::ok)
                return r;
            if(from!=from_end)
                return std::codecvt_base::partial;
            return r;
        }

        //encoding// Implementation for UTF-16
        //
        std::codecvt_base::result
        do_real_out(std::mbstate_t &state,
                    uint16_t const *from,
                    uint16_t const *from_end,
                    uint16_t const *&from_next,
                    char *to,
                    char *to_end,
                    char *&to_next) const
        {
            std::auto_ptr<base_converter> cvtp;
            base_converter *cvt = 0;
            if(cvt_->is_thread_safe()) {
                cvt = cvt_.get();
            }
            else {
                cvtp.reset(cvt_->clone());
                cvt = cvtp.get();
            }
            std::codecvt_base::result r=std::codecvt_base::ok;
            while(to < to_end && from < from_end)
            {
                uint32_t ch=*from;
                if(0xD800 <= ch && ch<=0xDFFF) {
                    r=std::codecvt_base::error;
                    // Can't handle surragates
                    break;
                }
                        
                uint32_t len=cvt->from_unicode(ch,to,to_end);
                if(len==base_converter::illegal) {
                    r=std::codecvt_base::error;
                    break;
                }
                if(len==base_converter::incomplete) {
                    r=std::codecvt_base::partial;
                    break;
                }
                to+=len;
                from++;
            }
            from_next=from;
            to_next=to;
            if(r!=std::codecvt_base::ok)
                return r;
            if(from!=from_end)
                return std::codecvt_base::partial;
            return r;
        }
        
        int max_len_;
        std::auto_ptr<base_converter> cvt_;

    };
    
    template<>
    class code_converter<char> : public std::codecvt<char,char,mbstate_t>
    {
    public:
        code_converter(std::auto_ptr<base_converter> cvt,size_t refs = 0) : 
            std::codecvt<char,char,mbstate_t>(refs)  
        {
        }
    };


    std::locale create_codecvt(std::locale const &in,std::auto_ptr<base_converter> cvt,character_facet_type type)
    {
        switch(type) {
        case char_facet:
            return std::locale(in,new code_converter<char>(cvt));
        #ifndef BOOST_NO_STD_WSTRING
        case wchar_t_facet:
            return std::locale(in,new code_converter<wchar_t>(cvt));
        #endif
        #if defined(BOOST_HAS_CHAR16_T) && !defined(BOOST_NO_CHAR16_T_CODECVT)
        case char16_t_facet:
            return std::locale(in,new code_converter<char16_t>(cvt));
        #endif
        #if defined(BOOST_HAS_CHAR32_T) && !defined(BOOST_NO_CHAR32_T_CODECVT)
        case char32_t_facet:
            return std::locale(in,new code_converter<char32_t>(cvt));
        #endif
        default:
            return in;
        }
    }


} // util
} // locale 
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
