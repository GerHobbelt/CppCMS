//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_LOCALE_SOURCE
#include <boost/locale/codepage.hpp>
#include "conv.h"

#include <assert.h>
#include <iostream>

#ifdef BOOST_MSVC
#  pragma warning(disable : 4244) // loose data 
#endif

#ifndef NOMINMAX
# define NOMINMAX
#endif

#include <windows.h>

#include <vector>
namespace boost {
namespace locale {
namespace conv {
namespace impl {

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
        if(charset == "windows1255" || charset=="cp1255")
            return 1255;
        if(charset=="utf8")
            return 65001;
        if(charset == "iso88598")
            return 28598;
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

int main()
{
    try {
    using namespace boost::locale::conv;
    using namespace boost::locale::conv::impl;
    std::string shalom_utf8="שלום";
    std::string shalom_pease_utf8="\xFF\xFFשלום Мир world";
    std::wstring shalom_wchar_t=L"שלום";

    std::string shalom_1255="\xf9\xec\xe5\xed";

    {
        wconv_from_utf<char> utfc;
        wconv_from_utf<wchar_t> wcharc;
        assert(utfc.open("windows-1255",stop));
        assert(wcharc.open("windows-1255",stop));
        assert(utfc.convert(shalom_utf8.c_str(),shalom_utf8.c_str()+shalom_utf8.size()) == shalom_1255);
        assert(wcharc.convert(shalom_wchar_t.c_str(),shalom_wchar_t.c_str()+shalom_wchar_t.size()) == shalom_1255);
        assert(wcharc.open("utf-8",stop));
        assert(wcharc.convert(shalom_wchar_t.c_str(),shalom_wchar_t.c_str()+shalom_wchar_t.size()) == shalom_utf8);
        assert(utfc.open("windows-1255",skip));
        assert(utfc.convert(shalom_pease_utf8.c_str(),shalom_pease_utf8.c_str()+shalom_pease_utf8.size()) == shalom_1255+" ??? world");
    }

    std::cerr << "From Ok" << std::endl;

    {
        wconv_to_utf<char> utfc;
        wconv_to_utf<wchar_t> wcharc;
        assert(utfc.open("windows-1255",stop));
        assert(wcharc.open("windows-1255",stop));
        assert(utfc.convert(shalom_1255.c_str(),shalom_1255.c_str()+shalom_1255.size())==shalom_utf8);
        assert(wcharc.convert(shalom_1255.c_str(),shalom_1255.c_str()+shalom_1255.size())==shalom_wchar_t);
    }
    
    std::cerr << "To Ok" << std::endl;
    {
        wconv_between cvt;
        assert(cvt.open("utf-8","windows-1255",stop));
        assert(cvt.convert(shalom_1255.c_str(),shalom_1255.c_str()+shalom_1255.size())==shalom_utf8);
        assert(cvt.open("windows-1255","iso-8859-8",stop));
        assert(cvt.convert(shalom_1255.c_str(),shalom_1255.c_str()+shalom_1255.size())==shalom_1255);

    }
    
    std::cerr << "Between Ok" << std::endl;
    {
        std::basic_string<uint32_t> utf32;
        
        utf32+=uint32_t(0x2008A);
        utf32+=uint32_t(0x5e9);
        utf32+=uint32_t(0x61);

        std::string utf8="\xf0\xa0\x82\x8a\xd7\xa9\x61";
        wconv_to_utf<uint32_t> to;
        assert(to.open("UTF-8",stop));
        wconv_from_utf<uint32_t> from;
        assert(from.open("UTF-8",stop));
        assert(to.convert(utf8.c_str(),utf8.c_str()+utf8.size()) == utf32);
        assert(from.convert(utf32.c_str(),utf32.c_str()+utf32.size()) == utf8);
    }
    std::cerr << "UTF-32 Ok" << std::endl;
    }
    catch(std::exception const &e) {
        std::cerr << e.what() << std::endl;
    }
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
