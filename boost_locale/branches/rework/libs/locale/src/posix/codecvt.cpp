//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_LOCALE_SOURCE
#include <boost/locale/encoding.hpp>
#include "../encoding/conv.hpp"
#include "../util/codecvt_converter.hpp"
#include "all_generator.hpp"

#include <iconv.h>
#include <algorithm>

#include <vector>
namespace boost {
namespace locale {
namespace impl_posix {
    class mb2_iconv_converter : public util::base_converter {
    public:
       
        mb2_iconv_converter(std::string const &encoding) :
            encoding_(encoding),
            to_utf_((iconv_t)(-1)),
            from_utf_((iconv_t)(-1))
        {
        }
        
        virtual ~mb2_iconv_converter()
        {
            if(to_utf_ != (iconv_t)(-1))
                iconv_close(to_utf_);
            if(from_utf_ != (iconv_t)(-1))
                iconv_close(from_utf_);

        }

        virtual bool is_thread_safe() const
        {
            return false;
        }

        virtual mb2_iconv_converter *clone() const
        {
            return new mb2_iconv_converter(encoding_);
        }

        uint32_t to_unicode(char const *&begin,char const *end)
        {
            if(begin == end)
                return incomplete;
            if(*begin == 0) {
                begin++;
                return 0;
            }
            
            open(to_utf_,utf32_encoding(),encoding_.c_str());

            unsigned char seq0 = *begin;
            // single byte
            if(seq0 <= 0x7F) {
                char *inbuf = reinterpret_cast<char*>(&seq0);
                size_t insize = 1;
                uint32_t result;
                size_t outsize = 4;
                char *outbuf = reinterpret_cast<char*>(&result);
                iconv(to_utf_,&inbuf,&insize,&outbuf,&outsize);
                if(outsize != 0 || insize != 0)
                    return illegal;
                begin++;
                return result;
            }
            {   // maybe single byte
                char inseq[2] = {seq0 , 0};
                char *inbuf = inseq;
                size_t insize = 2;
                uint32_t result[2] = { illegal, illegal };
                size_t outsize = 8;
                char *outbuf = reinterpret_cast<char*>(result);
                iconv(to_utf_,&inbuf,&insize,&outbuf,&outsize);
                if(outsize == 0 && insize == 0 && result[1]==0 ) {
                    begin++;
                    return result[0];
                }
            }
            {   // maybe illegal or may be double byte
                if(begin+1 == end)
                    return incomplete;
                char inseq[3] = {seq0 , begin[1], 0};
                char *inbuf = inseq;
                size_t insize = 3;
                uint32_t result[2] = { illegal, illegal };
                size_t outsize = 8;
                char *outbuf = reinterpret_cast<char*>(result);
                iconv(to_utf_,&inbuf,&insize,&outbuf,&outsize);
                if(outsize == 0 && insize == 0 && result[1]==0 ) {
                    begin+=2;
                    return result[0];
                }
                return illegal;
            }
        }

        uint32_t from_unicode(uint32_t cp,char *begin,char const *end)
        {
            if(cp == 0) {
                if(begin!=end) {
                    *begin = 0;
                    return 1;
                }
                else {
                    return incomplete;
                }
            }

            open(from_utf_,encoding_.c_str(),utf32_encoding());

            uint32_t codepoints[2] = {cp,0};
            char *inbuf = reinterpret_cast<char *>(codepoints);
            size_t insize = sizeof(codepoints);
            char outseq[3] = {0};
            char *outbuf = outseq;
            size_t outsize = 3;

            iconv(from_utf_,&inbuf,&insize,&outbuf,&outsize);

            if(insize != 0 || outsize > 1)
                return illegal;
            size_t len = 2 - outsize ;
            size_t reminder = end - begin;
            if(reminder < len)
                return incomplete;
            for(unsigned i=0;i<len;i++)
                *begin++ = outseq[i];
            return len;
        }

        void open(iconv_t &d,char const *to,char const *from)
        {
            if(d==(iconv_t)(-1))
                return;
            d=iconv_open(to,from);
        }

        static char const *utf32_encoding()
        {
            union { char one; uint32_t value; } test;
            test.value = 1;
            if(test.one == 1)
                return "UTF-32LE";
            else
                return "UTF-32BE";
        }

        virtual int max_len() const
        {
            return 2;
        }

    private:
        std::string encoding_;
        iconv_t to_utf_;
        iconv_t from_utf_;
    };

    std::locale create_codecvt(std::locale const &in,std::string const &encoding,character_facet_type type)
    {
        std::auto_ptr<util::base_converter> cvt;
        if(conv::impl::normalize_encoding(encoding.c_str())=="utf8")
            cvt = util::create_utf8_converter(); 
        else {
            cvt = util::create_simple_converter(encoding);
            if(!cvt.get())
                cvt.reset(new mb2_iconv_converter(encoding));
        }
        return util::create_codecvt(in,cvt,type);
    }

} // impl_posix
} // locale 
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
