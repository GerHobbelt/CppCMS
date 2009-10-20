#ifndef BOOST_SRC_LOCALE_HPP
#define BOOST_SRC_LOCALE_HPP
#include <unicode/unistr.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include <boost/noncopyable.hpp>

#include <string>
#include "icu_util.hpp"

namespace boost {
namespace locale {
namespace impl {

       
    template<typename CharType,int char_size = sizeof(CharType) >
    class icu_std_converter {
    public:
        typedef CharType char_type;
        typedef std::basic_string<char_type> string_type;

        icu_std_converter(std::string charset);         
        icu::UnicodeString icu(char_type const *begin,char_type const *end) const;
        string_type std(icu::UnicodeString const &str) const;
    };

    template<typename CharType>
    class icu_std_converter<CharType,1> {
    public:
        typedef CharType char_type;
        typedef std::basic_string<char_type> string_type;

        
        icu::UnicodeString icu(char_type const *vb,char_type const *ve) const
        {
            char const *begin=reinterpret_cast<char const *>(vb);
            char const *end=reinterpret_cast<char const *>(ve);
            icu::UnicodeString tmp(begin,end-begin,charset_.c_str());
            return tmp;
        }
        
        string_type std(icu::UnicodeString const &str) const
        {
            uconv cvt(charset_);
            return cvt.go(str.getBuffer(),str.length(),max_len_);
        }

        icu_std_converter(std::string charset) : charset_(charset)
        {
            uconv cvt(charset_);
            max_len_=cvt.max_char_size();
        }

        struct uconv : public boost::noncopyable {
        public:
            uconv(std::string const &charset) 
            {
                UErrorCode err=U_ZERO_ERROR;
                cvt_ = ucnv_open(charset.c_str(),&err);
                if(!cvt_)
                    throw_icu_error(err);
            }

            int max_char_size()
            {
                return ucnv_getMaxCharSize(cvt_);
            }

            string_type go(UChar const *buf,int length,int max_size)
            {
                string_type res;
                res.resize(UCNV_GET_MAX_BYTES_FOR_STRING(length,max_size));
                char *ptr=reinterpret_cast<char *>(&res[0]);
                UErrorCode err=U_ZERO_ERROR;
                int n = ucnv_fromUChars(cvt_,ptr,res.size(),buf,length,&err);
                check_and_throw_icu_error(err);
                res.resize(n);
                return res;
            }

            ~uconv()
            {
                ucnv_close(cvt_);
            }
                
        private:
            
            UConverter *cvt_;
        };

    private:
        int max_len_;
        std::string charset_;
    };
   
    template<typename CharType>
    class icu_std_converter<CharType,2> {
    public:
        typedef CharType char_type;
        typedef std::basic_string<char_type> string_type;

        
        icu::UnicodeString icu(char_type const *vb,char_type const *ve) const
        {
            UChar const *begin=reinterpret_cast<UChar const *>(vb);
            UChar const *end=reinterpret_cast<UChar const *>(ve);
            icu::UnicodeString tmp(begin,end-begin);
            return tmp;

        }

        string_type std(icu::UnicodeString const &str) const
        {
            char_type const *ptr=reinterpret_cast<char_type const *>(str.getBuffer());
            return string_type(ptr,str.length());
        }
        
        icu_std_converter(std::string charset) {}

    };
    
    template<typename CharType>
    class icu_std_converter<CharType,4> {
    public:

        typedef CharType char_type;
        typedef std::basic_string<char_type> string_type;

        
        icu::UnicodeString icu(char_type const *vb,char_type const *ve) const
        {
            UChar32 const *begin=reinterpret_cast<UChar32 const *>(vb);
            UChar32 const *end  =reinterpret_cast<UChar32 const *>(ve);

            icu::UnicodeString tmp(end-begin,0,0); // make inital capacity
            while(begin!=end)
                tmp.append(*begin++);
            return tmp;

        }

        string_type std(icu::UnicodeString const &str) const
        {
            string_type tmp;
            tmp.resize(str.length());
            UChar32 *ptr=reinterpret_cast<UChar32 *>(&tmp[0]);

            int32_t len=0;
            
            UErrorCode code=U_ZERO_ERROR;
            u_strToUTF32(ptr,tmp.size(),&len,str.getBuffer(),str.length(),&code);

            check_and_throw_icu_error(code);

            tmp.resize(len);

            return tmp;
        }

        icu_std_converter(std::string charset) {}

    };
} /// impl
} //  locale
} // boost

#endif


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
