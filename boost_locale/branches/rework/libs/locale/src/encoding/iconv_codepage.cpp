//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_LOCALE_SOURCE
#include <boost/locale/codepage.hpp>
#include <iconv.h>


#ifdef BOOST_MSVC
#  pragma warning(disable : 4244) // loose data 
#endif

#include <vector>
namespace boost {
namespace locale {
    namespace conv {

        namespace {
            
            extern "C" {
                typedef size_t gnu_iconv_type(iconv_t,char const **,size_t *,char **,size_t *);
                typedef size_t std_iconv_type(iconv_t,char **,size_t *,char **,size_t *);
            }
            
            template<typename CharType>
            char const *utf_name<>()
            {
                union {
                    char first;
                    uint16_t u16;
                    uint32_t u32;
                } v;
                if(sizeof(CharType) == 1) {
                    return "UTF-8";
                }
                else if(sizeof(CharType) == 2) {
                    v.u16 = 1;
                    if(first == 1) {
                        return "UTF-16LE";
                    }
                    else {
                        return "UTF-16BE";
                    }
                }
                else if(sizeof(CharType) == 4) {
                    v.u32 = 1;
                    if(first == 1) {
                        return "UTF-32LE";
                    }
                    else {
                        return "UTF-32BE";
                    }

                }
                else {
                    return "Unknown Character Encoding";
                }
            }

        }

        class iconverter {
        public:

            iconverter(char const *to,char const *from)
            {
                cvt_ = iconv_open(to,from);
                if(icv==(iconv_t)(-1)) {
                    throw std::runtime_error("Unsupported charset:" + charset);
                }
            }
            
            size_t conv(char const **inbuf,size_t *inchar_left,
                        char **outbuf,size_t *outchar_left)
            {
                return do_conv(iconv,inbuf,inchar_left,outbuf,outchar_left);
            }
        private:
            size_t do_conv(gnu_iconv_type *real_conv,char const **inbuf,size_t *inchar_left,char **outbuf,size_t *outchar_left)
            {
                return real_conv(cvt_,inbuf,inchar_left,outbuf,outchar_left);
            }

            size_t do_conv(std_iconv_type *real_conv,char const **inbuf,size_t *inchar_left,char **outbuf,size_t *outchar_left)
            {
                return real_conv(cvt_,const_cast<char **>(inbuf),inchar_left,outbuf,outchar_left);
            }

            ~iconverter()
            {
                iconv_close(cvt_);
            }
            iconv_t cvt_;
        };

        template<typename CharType>
        std::basic_string<CharType> to_utf_impl(char const *begin,char const *end,std::string const &charset,method_type how=default_method)
        {
            std::basic_string<CharType> converted;
            iconverter cv(utf_name<CharType>,charset.c_str());
            size_t inleft = end - begin;
            static const size_t outbuf_size = 32;
            CharType outbuf[outbuf_size];

            char *coutbuf = reinterpret_cast<char *>(outbuf);
            char *coutbuf_start = coutbuf;
            size_t outsize = sizeof(outbuf);

            size_t res;
            for(;;) {
                if(inleft > 0) {
                    res = cv.conv(&begin,inleft,coutbuf,outsize);
                }
                else {
                    res = cv.conv(0,0,coutbuf,outsize);
                }
                if(res == (size_t)(-1)) {
                    if(errno == E2BIG) {
                        size_t converted_chars = coutbuf - coutbuf_start;
                        size_t full_chars = converted_chars / sizeof(CharType);
                        size_t full_size = full_chars * sizeof(CharType);
                        size_t diff = converted_chars - full_size;

                        converted.append(outbuf,full_chars);

                        if(diff > 0) {
                            memmove(coutbuf_start,coutbuf_start+full_size,diff);
                            coutbuff = coutbuf_start + diff;
                            outsize = sizeof(outbuf) - diff;
                        }
                    }
                    else
                        throw conversion_error();

                }
                else if(res == 0) {
                    break;
                }
            }

            return converted;
        }

        template<typename CharType>
        std::string from_utf_impl(CharType const *begin,CharType const *end,std::string const &charset,method_type how=default_method)
        {
            impl_icu::icu_std_converter<CharType> cvt_from("UTF-8",how == skip ? impl_icu::cvt_skip : impl_icu::cvt_stop);
            impl_icu::icu_std_converter<char> cvt_to(charset,how == skip ? impl_icu::cvt_skip : impl_icu::cvt_stop);
            try {
                return cvt_to.std(cvt_from.icu(begin,end));
            }
            catch(std::exception const &/*e*/) {
                throw conversion_error();
            }
        }

        template<>
        BOOST_LOCALE_DECL std::basic_string<char> to_utf(char const *begin,char const *end,std::string const &charset,method_type how)
        {
            return to_utf_impl<char>(begin,end,charset,how);
        }

        template<>
        BOOST_LOCALE_DECL std::string from_utf(char const *begin,char const *end,std::string const &charset,method_type how)
        {
            return from_utf_impl<char>(begin,end,charset,how);
        }

        #if !defined(BOOST_NO_STD_WSTRING) || defined(BOOST_WINDOWS)
        template<>
        BOOST_LOCALE_DECL std::basic_string<wchar_t> to_utf(char const *begin,char const *end,std::string const &charset,method_type how)
        {
            return to_utf_impl<wchar_t>(begin,end,charset,how);
        }

        template<>
        BOOST_LOCALE_DECL std::string from_utf(wchar_t const *begin,wchar_t const *end,std::string const &charset,method_type how)
        {
            return from_utf_impl<wchar_t>(begin,end,charset,how);
        }
        #endif

        #ifdef BOOST_HAS_CHAR16_T
        template<>
        BOOST_LOCALE_DECL std::basic_string<char16_t> to_utf(char const *begin,char const *end,std::string const &charset,method_type how)
        {
            return to_utf_impl<char16_t>(begin,end,charset,how);
        }


        template<>
        BOOST_LOCALE_DECL std::string from_utf(char16_t const *begin,char16_t const *end,std::string const &charset,method_type how)
        {
            return from_utf_impl<char16_t>(begin,end,charset,how);
        }
        #endif

        #ifdef BOOST_HAS_CHAR32_T
        template<>
        BOOST_LOCALE_DECL std::basic_string<char32_t> to_utf(char const *begin,char const *end,std::string const &charset,method_type how)
        {
            return to_utf_impl<char32_t>(begin,end,charset,how);
        }

        template<>
        BOOST_LOCALE_DECL std::string from_utf(char32_t const *begin,char32_t const *end,std::string const &charset,method_type how)
        {
            return from_utf_impl<char32_t>(begin,end,charset,how);
        }
        #endif
    } // conv

} // locale 
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
