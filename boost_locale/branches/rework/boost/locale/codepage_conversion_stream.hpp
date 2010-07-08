//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_CODEPAGE_HPP_INCLUDED
#define BOOST_LOCALE_CODEPAGE_HPP_INCLUDED

#include <istream>
#include <ostream>
#include <streambuf>

#include <boost/locale/codepage.hpp>

namespace boost {
    namespace locale {
        namespace conv {

            template<typename FromChar,typename ToChar>
            class conversion_stream_base :
                public std::basic_istream<FromChar>,
                public std::basic_ostream<ToChar>
            {
                conversion_stream_base(conversion_stream_base const &);
                void operator=(conversion_stream_base const &);
            public:
                typedef std::basic_istream<FromChar> istream_type;
                typedef std::basic_ostream<ToChar> ostream_type;
                typedef codepage_converter<FromChar,ToChar> converter_type;
                
                virtual ~conversion_stream_base()
                {
                    delete converter_;
                }

                void in_method(method_type m)
                {
                    converter_->in_method(m);
                }

                void out_method(method_type m)
                {
                    converter_->out_method(m);
                }
                
                std::codecvt_base::result status()
                {
                    return converter_->status();
                }

                conversion_stream_base(converter_type *cvt) : converter_(0)
                {
                    init(cvt);
                }

            protected:
                conversion_stream_base() : converter_(0)
                {
                }
                void init(converter_type *cvt)
                {
                    converter_ = cvt;
                    istream_type::init(converter_->in());
                    ostream_type::init(converter_->out());

                }
            private:
                converter_type *converter_;
            };


            template<typename FromChar,typename ToChar>
            class conversion_stream;

            template<typename FromChar>
            class basic_conversion_stream<FromChar,char> : public conversion_stream_base<FromChar,char>
            {
            public:
                basic_conversion_stream(std::locale const &source_locale,std::string const &target_encoding)
                {
                    init(std::use_facet<codepage_facet<FromChar> >(source_locale).from_iternal(target_encoding));
                }
            };
            
            template<typename ToChar>
            class basic_conversion_stream<char,ToChar> : public conversion_stream_base<char,ToChar>
            {
            public:
                basic_conversion_stream(std::string const &source_encoding,std::locale const &target_locale)
                {
                    init(std::use_facet<codepage_facet<ToChar> >(target_locale).to_iternal(source_encoding));
                }
            };
            
            template<>
            class basic_conversion_stream<char,char> : public conversion_stream_base<char,char>
            {
            public:
                basic_conversion_stream(std::string source_encoding,std::locale const &target_locale)
                {
                    init(std::use_facet<codepage_facet<ToChar> >(target_locale).to_iternal(source_encoding));
                }
                basic_conversion_stream(std::locale const &source_locale,std::string target_encoding)
                {
                    init(std::use_facet<codepage_facet<ToChar> >(source_locale).from_iternal(target_encoding));
                }
            };

            typedef basic_conversion_stream<char,char> conversion_iostream;
            
            #ifndef BOOST_NO_STD_WSTRING
            
            typedef basic_conversion_stream<wchar_t,char> conversion_wistream;
            typedef basic_conversion_stream<char,wchar_t> conversion_wostream;
            
            #endif

            #ifdef BOOST_HAS_CHAR16_T
            typedef basic_conversion_stream<char16_t,char> conversion_u16istream;
            typedef basic_conversion_stream<char,char16_t> conversion_u16ostream;
            #endif
            
            #ifdef BOOST_HAS_CHAR32_T
            typedef basic_conversion_stream<char32_t,char> conversion_u32istream;
            typedef basic_conversion_stream<char,char32_t> conversion_u32ostream;
            #endif

        } // conv
    } // locale
} // boost


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

