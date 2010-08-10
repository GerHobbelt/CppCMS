//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_LOCALE_IMPL_WCONV_CODEPAGE_HPP
#define BOOST_LOCALE_IMPL_WCONV_CODEPAGE_HPP


#include <boost/locale/codepage.hpp>
#include <algorithm>
#include "conv.hpp"

#ifndef NOMINMAX
# define NOMINMAX
#endif
#include <windows.h>
#include <vector>


namespace boost {
namespace locale {
namespace conv {
namespace impl {
    
    struct windows_encoding {
        char const *name;
        unsigned codepage;
    };

    bool operator<(windows_encoding const &l,windows_encoding const &r)
    {
        return strcmp(l.name,r.name) < 0;
    }

    windows_encoding all_windows_encodings[] = {
        { "big5",       950 },
        { "eucjp",      20932 },
        { "euckr",      51949 },
        { "gb18030",    54936 },
        { "gb2312",     936 },
        { "iso2022jp",  50220 },
        { "iso2022kr",  50225 },
        { "iso88591",   28591 },
        { "iso885913",  28603 },
        { "iso885915",  28605 },
        { "iso88592",   28592 },
        { "iso88593",   28593 },
        { "iso88594",   28594 },
        { "iso88595",   28595 },
        { "iso88596",   28596 },
        { "iso88597",   28597 },
        { "iso88598",   28598 },
        { "iso88599",   28599 },
        { "koi8r",      20866 },
        { "koi8u",      21866 },
        { "shiftjis",   932 },
        { "sjis",       932 },
        { "utf8",       65001 },
        { "windows1250",        1250 },
        { "windows1251",        1251 },
        { "windows1252",        1252 },
        { "windows1253",        1253 },
        { "windows1254",        1254 },
        { "windows1255",        1255 },
        { "windows1256",        1256 },
        { "windows1257",        1257 },
        { "windows874", 874 },
    };

    int encoding_to_windows_codepage(char const *ccharset)
    {
        std::string charset;
        charset.reserve(strlen(ccharset));
        while(*ccharset!=0) {
            char c=*ccharset++;
            if('0' <= c && c<= '9')
                charset+=c;
            else if('a' <=c && c <='z')
                charset+=c;
            else if('A' <=c && c <='Z')
                charset+=char(c-'A'+'a');
        }
        windows_encoding ref;
        ref.name = charset.c_str();
        size_t n = sizeof(all_windows_encodings)/sizeof(all_windows_encodings[0]);
        windows_encoding *begin = all_windows_encodings;
        windows_encoding *end = all_windows_encodings + n;
        windows_encoding *ptr = std::lower_bound(begin,end,ref);
        if(ptr!=end && strcmp(ptr->name,charset.c_str())==0)
            return ptr->codepage;
        return -1;
    }

    bool validate_utf16(uint16_t const *str,unsigned len)
    {
        for(unsigned i=0;i<len;i++) {
           if(0xD800 <= str[i] && str[i]<= 0xDBFF) {
               i++;
               if(i>=len)
                   return false;
                if(0xDC00 <= str[i] && str[i]<=0xDFFF)
                    continue;
                return false;
           }
           else if(0xDC00 <= str[i] && str[i]<=0xDFFF)
               return false;
        }
        return true;
    }

    class wconv_between : public converter_between {
    public:
        wconv_between() : 
            how_(skip),
            to_code_page_ (-1),
            from_code_page_ ( -1)
        {
        }
        bool open(char const *to_charset,char const *from_charset,method_type how)
        {
            to_code_page_ = encoding_to_windows_codepage(to_charset);
            from_code_page_ = encoding_to_windows_codepage(from_charset);
            if(to_code_page_ == -1 || from_code_page_ == -1)
                return false;
            return true;
        }
        virtual std::string convert(char const *begin,char const *end)
        {
            DWORD flags = how_ == skip ? 0 : MB_ERR_INVALID_CHARS;
            if(50220 <= from_code_page_ && from_code_page_ <= 50229)
                flags = 0;
            
            int n = MultiByteToWideChar(from_code_page_,flags,begin,end-begin,0,0);
            if(n == 0)
                throw conversion_error();
            std::vector<wchar_t> buf(n);
            if(MultiByteToWideChar(from_code_page_,flags,begin,end-begin,&buf.front(),buf.size())==0)
                throw conversion_error();
            n = WideCharToMultiByte(to_code_page_,0,&buf[0],buf.size(),0,0,0,0);
            std::vector<char> cbuf(n);
            BOOL substitute = FALSE;
            BOOL *substitute_ptr = to_code_page_ == 65001 || to_code_page_ == 65000 ? 0 : &substitute;
            if(WideCharToMultiByte(to_code_page_,0,&buf[0],buf.size(),&cbuf[0],n,0,substitute_ptr)==0)
                throw conversion_error();
            if(how_ == stop && substitute)
                throw conversion_error();
            return std::string(&cbuf[0],n);
        }
    private:
        method_type how_;
        int to_code_page_;
        int from_code_page_;
    };
    
    template<typename CharType,int size = sizeof(CharType) >
    class wconv_to_utf;

    template<typename CharType,int size = sizeof(CharType) >
    class wconv_from_utf;

    template<>
    class wconv_to_utf<char,1> : public  converter_to_utf<char> , public wconv_between {
    public:
        virtual bool open(char const *cs,method_type how) 
        {
            return wconv_between::open("UTF-8",cs,how);
        }
        virtual std::string convert(char const *begin,char const *end)
        {
            return wconv_between::convert(begin,end);
        }
    };
    
    template<>
    class wconv_from_utf<char,1> : public  converter_from_utf<char> , public wconv_between {
    public:
        virtual bool open(char const *cs,method_type how) 
        {
            return wconv_between::open(cs,"UTF-8",how);
        }
        virtual std::string convert(char const *begin,char const *end)
        {
            return wconv_between::convert(begin,end);
        }
    };
    
    template<typename CharType>
    class wconv_to_utf<CharType,2> : public converter_to_utf<CharType> {
    public:
        typedef CharType char_type;

        typedef std::basic_string<char_type> string_type;

        wconv_to_utf() : 
            how_(skip),
            code_page_(-1)
        {
        }

        virtual bool open(char const *charset,method_type how)
        {
            how_ = how;
            code_page_ = encoding_to_windows_codepage(charset);
            return code_page_ != -1;
        }

        virtual string_type convert(char const *begin,char const *end) 
        {
            DWORD flags = how_ == skip ? 0 : MB_ERR_INVALID_CHARS;
            if(50220 <= code_page_ && code_page_ <= 50229)
                flags = 0;
            
            int n = MultiByteToWideChar(code_page_,flags,begin,end-begin,0,0);
            if(n == 0) {
                throw conversion_error();
            }
            std::vector<wchar_t> buf(n);
            if(MultiByteToWideChar(code_page_,flags,begin,end-begin,&buf.front(),buf.size())==0) {
                throw conversion_error();
            }
            string_type res;
            res.assign(reinterpret_cast<char_type *>(&buf[0]),n);
            return res;
        }

    private:
        method_type how_;
        int code_page_;
    };
  
    template<typename CharType>
    class wconv_from_utf<CharType,2> : public converter_from_utf<CharType> {
    public:
        typedef CharType char_type;

        typedef std::basic_string<char_type> string_type;

        wconv_from_utf() : 
            how_(skip),
            code_page_(-1)
        {
        }

        virtual bool open(char const *charset,method_type how)
        {
            how_ = how;
            code_page_ = encoding_to_windows_codepage(charset);
            return code_page_ != -1;
        }

        virtual std::string convert(CharType const *begin,CharType const *end) 
        {
            if(how_ == stop && !validate_utf16(reinterpret_cast<uint16_t const *>(begin),end-begin)) {
                throw conversion_error();
            }

            wchar_t const *ptr = reinterpret_cast<wchar_t const *>(begin);
            int n = WideCharToMultiByte(code_page_,0,ptr,end-begin,0,0,0,0);
            if(n==0)
                throw conversion_error();
            std::vector<char> cbuf(n);
            BOOL substitute = FALSE;
            BOOL *substitute_ptr = code_page_ == 65001 || code_page_ == 65000 ? 0 : &substitute;
            if(WideCharToMultiByte(code_page_,0,ptr,end-begin,&cbuf[0],n,0,substitute_ptr)==0) {
                throw conversion_error();
            }
            if(how_ == stop && substitute) {
                throw conversion_error();
            }
            return std::string(&cbuf[0],n);
        }

    private:
        method_type how_;
        int code_page_;
    };



    template<typename CharType>
    class wconv_to_utf<CharType,4> : public converter_to_utf<CharType> {
    public:
        typedef CharType char_type;

        typedef std::basic_string<char_type> string_type;

        wconv_to_utf() : 
            how_(skip),
            code_page_(-1)
        {
        }

        virtual bool open(char const *charset,method_type how)
        {
            how_ = how;
            code_page_ = encoding_to_windows_codepage(charset);
            return code_page_ != -1;
        }

        virtual string_type convert(char const *begin,char const *end) 
        {
            DWORD flags = how_ == skip ? 0 : MB_ERR_INVALID_CHARS;
            if(50220 <= code_page_ && code_page_ <= 50229)
                flags = 0;
            
            int n = MultiByteToWideChar(code_page_,flags,begin,end-begin,0,0);
            if(n == 0) {
                throw conversion_error();
            }
            std::vector<wchar_t> buf(n);
            if(MultiByteToWideChar(code_page_,flags,begin,end-begin,&buf.front(),buf.size())==0) {
                throw conversion_error();
            }
            string_type res;
            res.reserve(n);
            for(int i=0;i<n;i++) {
                wchar_t cur = buf[i];
                if(0xD800 <= cur && cur<= 0xDBFF) {
                    i++;
                    if(i>=n)
                        throw conversion_error();
                    if(0xDC00 <= buf[i] && buf[i]<=0xDFFF) {
                        uint32_t w1 = cur;
                        uint32_t w2 = buf[i];
                        uint32_t norm = ((uint32_t(w1 & 0x3FF) << 10) | (w2 & 0x3FF)) + 0x10000;
                        res+=char_type(norm);
                    }
                    else 
                        throw conversion_error();
                }
                else if(0xDC00 <= cur && cur<=0xDFFF)
                    throw conversion_error();
                else
                    res+=char_type(cur);
            }
            return res;
        }
    private:
        method_type how_;
        int code_page_;
    };
  
    template<typename CharType>
    class wconv_from_utf<CharType,4> : public converter_from_utf<CharType> {
    public:
        typedef CharType char_type;

        typedef std::basic_string<char_type> string_type;

        wconv_from_utf() : 
            how_(skip),
            code_page_(-1)
        {
        }

        virtual bool open(char const *charset,method_type how)
        {
            how_ = how;
            code_page_ = encoding_to_windows_codepage(charset);
            return code_page_ != -1;
        }

        virtual std::string convert(CharType const *begin,CharType const *end) 
        {
            std::wstring tmp;
            tmp.reserve(end-begin);
            while(begin!=end) {
                uint32_t cur = *begin++;
                if(cur > 0x10FFFF  || (0xD800 <=cur && cur <=0xDFFF)) {
                    if(how_ == skip)
                        continue;
                    else
                        throw conversion_error();
                }
                if(cur > 0xFFFF) {
                    uint32_t u = cur - 0x10000;
                    wchar_t first  = 0xD800 | (u>>10);
                    wchar_t second = 0xDC00 | (u & 0x3FF);
                    tmp+=first;
                    tmp+=second;
                }
                else {
                    tmp+=wchar_t(cur);
                }
            }

            int n = WideCharToMultiByte(code_page_,0,tmp.c_str(),tmp.size(),0,0,0,0);
            if(n==0)
                throw conversion_error();
            std::vector<char> cbuf(n);
            BOOL substitute = FALSE;
            BOOL *substitute_ptr = code_page_ == 65001 || code_page_ == 65000 ? 0 : &substitute;
            if(WideCharToMultiByte(code_page_,0,tmp.c_str(),tmp.size(),&cbuf[0],n,0,substitute_ptr)==0) {
                throw conversion_error();
            }
            if(how_ == stop && substitute) {
                throw conversion_error();
            }
            return std::string(&cbuf[0],n);
        }

    private:
        method_type how_;
        int code_page_;
    };





} // impl
} // conv
} // locale 
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
