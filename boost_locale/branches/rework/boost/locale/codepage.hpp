//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_CODEPAGE_HPP_INCLUDED
#define BOOST_LOCALE_CODEPAGE_HPP_INCLUDED

#include <boost/locale/config.hpp>
#ifdef BOOST_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4275 4251 4231 4660)
#endif
#include <boost/locale/info.hpp>
#include <boost/cstdint.hpp>
#include <stdexcept>
#include <iosfwd>
#include <istream>
#include <ostream>


namespace boost {
    namespace locale {

        ///
        /// \brief Namespace that contains all functions related to character set conversion
        ///
        namespace conv {
            ///
            /// \defgroup codepage Character conversion functions
            ///
            /// @{


            ///
            /// enum that defines conversion policy
            ///
            typedef enum {
                stop            = 0,    ///< Stop conversion and report error
                skip            = 1,    ///< Skip illegal/unconvertable characters
                default_method  = stop  ///< Default is stop conversion
            } method_type;

            
            ///
            /// \brief this class is created by the locale facet to convert one encoding to enother.
            ///
            /// It has two connected stream buffers - input stream buffer for consuming input text
            /// and output stream buffer for reading converted data.
            ///
            template<typename FromChar,typename ToChar>
            class codepage_converter {
                codepage_converter(codepage_converter const &);
                void operator=(codepage_converter const &);
            public:

                typedef FromChar from_char_type;
                typedef ToChar to_char_type;

                typedef std::basic_streambuf<FromChar> from_streambuf_type;
                typedef std::basic_streambuf<FromChar> to_streambuf_type;

                virtual void in_method(method_type m) = 0;

                virtual void out_method(method_type m) = 0;

                
                virtual from_streambuf_type *in() = 0; 

                virtual to_streambuf_type *out() = 0;
                
                virtual std::codecvt_base::result status() = 0;

                virtual ~codepage_converter()
                {
                }
            };


            template<typename CharType>
            class codepage_facet : public std::codecvt<CharType,char,mbstate_t> {
            public:
                codepage_facet(size_t refs = 0) : 
                    std::codecvt<CharType,char,mbstate_t>(refs)
                {
                }

                codepage_converter<CharType,char> *from_internal(std::string const &external_encoding) const
                {
                    return do_to_internal(external_encoding);
                }

                codepage_converter<char,CharType> *to_internal(std::string const &external_encoding) const
                {
                    return do_from_internal(external_encoding);
                }

            protected:

                virtual codepage_converter<CharType,char> *do_to_internal(std::string const &from_encoding) const = 0;

                virtual codepage_converter<char,CharType> *do_from_internal(std::string const &from_encoding) const = 0;

                virtual int do_encoding() const throw() = 0;
                
                virtual int do_max_length() const throw() = 0;
                
                virtual bool do_always_noconv() const throw() = 0;
                
                virtual std::codecvt_base::result do_unshift(std::mbstate_t &s,char *from,char *to,char *&next) const = 0;
                
                virtual std::codecvt_base::result 
                do_in(  std::mbstate_t &state,
                    char const *from,
                    char const *from_end,
                    char const *&from_next,
                    CharType *uto,
                    CharType *uto_end,
                    CharType *&uto_next) const = 0;
                
                virtual int
                do_length(  std::mbstate_t &state,
                        char const *from,
                        char const *from_end,
                        size_t max) const = 0;
                
                virtual std::codecvt_base::result 
                do_out( std::mbstate_t &state,
                        CharType const *ufrom,
                        CharType const *ufrom_end,
                        CharType const *&ufrom_next,
                        char *to,
                        char *to_end,
                        char *&to_next) const = 0;
 
            };

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
                    if(!cvt) {
                        throw std::invalid_argument("Invalid or unsupported encoding");
                    }
                    converter_ = cvt;
                    istream_type::init(converter_->in());
                    ostream_type::init(converter_->out());

                }
            private:
                converter_type *converter_;
            };


            template<typename FromChar,typename ToChar>
            class basic_conversion_stream;

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
                    init(std::use_facet<codepage_facet<char> >(target_locale).to_internal(source_encoding));
                }
                basic_conversion_stream(std::locale const &source_locale,std::string target_encoding)
                {
                    init(std::use_facet<codepage_facet<char> >(source_locale).from_internal(target_encoding));
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
            
            ///
            /// \brief The excepton that is thrown in case of conversion error
            ///
            class conversion_error : public std::runtime_error {
            public:
                conversion_error() : std::runtime_error("Conversion failed") {}
            };

            BOOST_LOCALE_DECL 
            std::string convert(char const *begin,char const *end,std::string const &source_encoding,std::string const &target_encoding);
            
            inline std::string convert(std::string const &src,std::string const &source_encoding,std::string const &target_encoding)
            {
                return convert(src.c_str(),src.c_str()+src.size(),source_encoding,target_encoding);
            }

            inline std::string convert(char const *ptr,std::string const &source_encoding,std::string const &target_encoding)
            {
                char const *end = ptr;
                while(*end!=0)
                    end++;
                return convert(ptr,end,source_encoding,target_encoding);
            }

            template<typename CharType>
            std::basic_string<CharType> to_locale_encoding( char const *begin,
                                                            char const *end,
                                                            std::string const &source_encoding,
                                                            std::locale const &target_locale=std::locale(),
                                                            method_type how = default_method)
            {
                basic_conversion_stream<char,CharType> stream(source_encoding,target_locale);
                stream.in_method(how);
                stream.out_method(how);

                stream.write(begin,end-begin);
                if(stream.status()!=std::codecvt_base::ok)
                    throw conversion_error();
                std::basic_string<CharType> str;
                str.reserve(end-begin);
                CharType buf[64];
                while(!stream.eof()) {
                    stream.read(buf,64);
                    str.append(buf,stream.gcount());
                }
                if(stream.status()!=std::codecvt_base::ok)
                    throw conversion_error();
                return str;
            }

            template<typename CharType>
            std::basic_string<CharType> to_locale_encoding( std::string const &s,
                                                            std::string const &source_encoding,
                                                            std::locale const &target_locale=std::locale(),
                                                            method_type how = default_method)
            {
                return to_locale_encoding<CharType>(s.c_str(),s.c_str()+s.size(),source_encoding,target_locale,how);
            }
            
            template<typename CharType>
            std::basic_string<CharType> to_locale_encoding( char const *begin,
                                                            std::string const &source_encoding,
                                                            std::locale const &target_locale=std::locale(),
                                                            method_type how = default_method)
            {
                char const *end=begin;
                while(*end!=0)
                    end++;
                return to_locale_encoding<CharType>(begin,end,source_encoding,target_locale,how);
            }
            
            template<typename CharType>
            std::string from_locale_encoding(   CharType const *begin,
                                                CharType const *end,
                                                std::string const &target_encoding,
                                                std::locale const &source_locale=std::locale(),
                                                method_type how = default_method)
            {
                basic_conversion_stream<CharType,char> stream(source_locale,target_encoding);
                stream.in_method(how);
                stream.out_method(how);

                stream.write(begin,end-begin);
                if(stream.status()!=std::codecvt_base::ok)
                    throw conversion_error();
                std::string str;
                str.reserve(end-begin);
                char buf[64];
                while(!stream.eof()) {
                    stream.read(buf,64);
                    str.append(buf,stream.gcount());
                }
                if(stream.status()!=std::codecvt_base::ok)
                    throw conversion_error();
                return str;
            }

            template<typename CharType>
            std::string from_locale_encoding(   std::basic_string<CharType> const &str,
                                                std::string const &target_encoding,
                                                std::locale const &source_locale=std::locale(),
                                                method_type how = default_method)
            {
                return from_locale_encoding<CharType>(str.c_str(),str.c_str()+str.size(),target_encoding,source_locale,how);
            }

            template<typename CharType>
            std::string from_locale_encoding(   CharType const *begin,
                                                std::string const &target_encoding,
                                                std::locale const &source_locale=std::locale(),
                                                method_type how = default_method)
            {
                CharType const *end=begin;
                while(*end!=0)
                    end++;
                return from_locale_encoding<CharType>(begin,end,target_encoding,source_locale,how);
            }

            ///
            /// convert string to UTF string from text in range [begin,end) encoded with \a charset according to policy \a how
            ///
            template<typename CharType>
            std::basic_string<CharType> to_utf(char const *begin,char const *end,std::string const &charset,method_type how=default_method);

            ///
            /// convert UTF text in range [begin,end) to a text encoded with \a charset according to policy \a how
            ///
            template<typename CharType>
            std::string from_utf(CharType const *begin,CharType const *end,std::string const &charset,method_type how=default_method);

            ///
            /// convert string to UTF string from text in range [begin,end) encoded according to locale \a loc according to policy \a how
            ///
            template<typename CharType>
            std::basic_string<CharType> to_utf(char const *begin,char const *end,std::locale const &loc,method_type how=default_method)
            {
                return to_utf<CharType>(begin,end,std::use_facet<info>(loc).encoding(),how);
            }

            ///
            /// convert UTF text in range [begin,end) to a text encoded according to locale \a loc according to policy \a how
            ///
            template<typename CharType>
            std::string from_utf(CharType const *begin,CharType const *end,std::locale const &loc,method_type how=default_method)
            {
                return from_utf(begin,end,std::use_facet<info>(loc).encoding(),how);
            }

            ///
            /// convert a string \a text encoded with \a charset to UTF string
            ///
            
            template<typename CharType>
            std::basic_string<CharType> to_utf(std::string const &text,std::string const &charset,method_type how=default_method)
            {
                return to_utf<CharType>(text.c_str(),text.c_str()+text.size(),charset,how);
            }

            ///
            /// Convert a \a text from \a charset to UTF string
            ///
            template<typename CharType>
            std::string from_utf(std::basic_string<CharType> const &text,std::string const &charset,method_type how=default_method)
            {
                return from_utf(text.c_str(),text.c_str()+text.size(),charset,how);
            }

            ///
            /// Convert a \a text from \a charset to UTF string
            ///
            template<typename CharType>
            std::basic_string<CharType> to_utf(char const *text,std::string const &charset,method_type how=default_method)
            {
                char const *text_end = text;
                while(*text_end) 
                    text_end++;
                return to_utf<CharType>(text,text_end,charset,how);
            }

            ///
            /// Convert a \a text from UTF to \a charset
            ///
            template<typename CharType>
            std::string from_utf(CharType const *text,std::string const &charset,method_type how=default_method)
            {
                CharType const *text_end = text;
                while(*text_end) 
                    text_end++;
                return from_utf(text,text_end,charset,how);
            }

            ///
            /// Convert a \a text in locale encoding given by \a loc to UTF
            ///
            template<typename CharType>
            std::basic_string<CharType> to_utf(std::string const &text,std::locale const &loc,method_type how=default_method)
            {
                return to_utf<CharType>(text.c_str(),text.c_str()+text.size(),loc,how);
            }

            ///
            /// Convert a \a text in UTF to locale encoding given by \a loc
            ///
            template<typename CharType>
            std::string from_utf(std::basic_string<CharType> const &text,std::locale const &loc,method_type how=default_method)
            {
                return from_utf(text.c_str(),text.c_str()+text.size(),loc,how);
            }

            ///
            /// Convert a \a text in locale encoding given by \a loc to UTF
            ///
            template<typename CharType>
            std::basic_string<CharType> to_utf(char const *text,std::locale const &loc,method_type how=default_method)
            {
                char const *text_end = text;
                while(*text_end) 
                    text_end++;
                return to_utf<CharType>(text,text_end,loc,how);
            }

            ///
            /// Convert a \a text in UTF to locale encoding given by \a loc
            ///
            template<typename CharType>
            std::string from_utf(CharType const *text,std::locale const &loc,method_type how=default_method)
            {
                CharType const *text_end = text;
                while(*text_end) 
                    text_end++;
                return from_utf(text,text_end,loc,how);
            }

            /// @}

            /// \cond INTERNAL

            template<>
            BOOST_LOCALE_DECL std::basic_string<char> to_utf(char const *begin,char const *end,std::string const &charset,method_type how);

            template<>
            BOOST_LOCALE_DECL std::string from_utf(char const *begin,char const *end,std::string const &charset,method_type how);

            #if !defined(BOOST_NO_STD_WSTRING) || defined(BOOST_WINDOWS)
            template<>
            BOOST_LOCALE_DECL std::basic_string<wchar_t> to_utf(char const *begin,char const *end,std::string const &charset,method_type how);

            template<>
            BOOST_LOCALE_DECL std::string from_utf(wchar_t const *begin,wchar_t const *end,std::string const &charset,method_type how);
            #endif

            #ifdef BOOST_HAS_CHAR16_T
            template<>
            BOOST_LOCALE_DECL std::basic_string<char16_t> to_utf(char const *begin,char const *end,std::string const &charset,method_type how);

            template<>
            BOOST_LOCALE_DECL std::string from_utf(char16_t const *begin,char16_t const *end,std::string const &charset,method_type how);
            #endif

            #ifdef BOOST_HAS_CHAR32_T
            template<>
            BOOST_LOCALE_DECL std::basic_string<char32_t> to_utf(char const *begin,char const *end,std::string const &charset,method_type how);

            template<>
            BOOST_LOCALE_DECL std::string from_utf(char32_t const *begin,char32_t const *end,std::string const &charset,method_type how);
            #endif

            /// \endcond
        } // conv

    } // locale
} // boost

#ifdef BOOST_MSVC
#pragma warning(pop)
#endif

#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

