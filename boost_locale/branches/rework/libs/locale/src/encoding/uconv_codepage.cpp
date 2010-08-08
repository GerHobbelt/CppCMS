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
#include "../icu/uconv.hpp"
#include <unicode/ucnv.h>
#include <unicode/ucnv_err.h>

#include <assert.h>
#include <iostream>

#ifdef BOOST_MSVC
#  pragma warning(disable : 4244) // loose data 
#endif

#include "icu_util.hpp"
#include <vector>
namespace boost {
namespace locale {
namespace conv {
namespace impl {
    template<typename CharType>
    class uconv_to_utf : public converter_to_utf<CharType> {
    public:
        typedef CharType char_type;

        typedef std::basic_string<char_type> string_type;

        virtual bool open(char const *charset,method_type how)
        {
            close();
            try {
                cvt_from_.reset(new from_type(charset,how == skip ? impl_icu::cvt_skip : impl_icu::cvt_stop));
                cvt_to_.reset(new to_type("UTF-8",how == skip ? impl_icu::cvt_skip : impl_icu::cvt_stop));
            }
            catch(std::exception const &/*e*/) {
                close();
                return false;
            }
            return true;
        }
        void close()
        {
            cvt_from_.reset();
            cvt_to_.reset();
        }

        virtual string_type convert(char const *begin,char const *end) 
        {
            try {
                return cvt_to_->std(cvt_from_->icu(begin,end));
            }
            catch(std::exception const &/*e*/) {
                throw conversion_error();
            }
        }

    private:

        typedef impl_icu::icu_std_converter<char> from_type;
        typedef impl_icu::icu_std_converter<CharType> to_type;

        std::auto_ptr<from_type> cvt_from_;
        std::auto_ptr<to_type> cvt_to_;

    };
  
  
    template<typename CharType>
    class uconv_from_utf : public converter_from_utf<CharType> {
    public:
        typedef CharType char_type;
        virtual bool open(char const *charset,method_type how)
        {
            close();
            try {
                cvt_from_.reset(new from_type("UTF-8",how == skip ? impl_icu::cvt_skip : impl_icu::cvt_stop));
                cvt_to_.reset(new to_type(charset,how == skip ? impl_icu::cvt_skip : impl_icu::cvt_stop));
            }
            catch(std::exception const &/*e*/) {
                close();
                return false;
            }
            return true;
        }
        void close()
        {
            cvt_from_.reset();
            cvt_to_.reset();
        }

        virtual std::string convert(CharType const *begin,CharType const *end) 
        {
            try {
                return cvt_to_->std(cvt_from_->icu(begin,end));
            }
            catch(std::exception const &/*e*/) {
                throw conversion_error();
            }
        }

    private:

        typedef impl_icu::icu_std_converter<CharType> from_type;
        typedef impl_icu::icu_std_converter<char> to_type;

        std::auto_ptr<from_type> cvt_from_;
        std::auto_ptr<to_type> cvt_to_;

    };
} // impl
} // conv
} // locale 
} // boost

int main()
{
    using namespace boost::locale::conv;
    using namespace boost::locale::conv::impl;
    std::string shalom_utf8="שלום";
    std::string shalom_pease_utf8="\xFF\xFFשלום Мир world";
    std::wstring shalom_wchar_t=L"שלום";
    std::string shalom_1255="\xf9\xec\xe5\xed";

    {
        uconv_from_utf<char> utfc;
        uconv_from_utf<wchar_t> wcharc;
        assert(utfc.open("windows-1255",stop));
        assert(wcharc.open("windows-1255",stop));
        assert(utfc.convert(shalom_utf8.c_str(),shalom_utf8.c_str()+shalom_utf8.size()) == shalom_1255);
        assert(wcharc.convert(shalom_wchar_t.c_str(),shalom_wchar_t.c_str()+shalom_wchar_t.size()) == shalom_1255);
        assert(wcharc.open("utf-8",stop));
        assert(wcharc.convert(shalom_wchar_t.c_str(),shalom_wchar_t.c_str()+shalom_wchar_t.size()) == shalom_utf8);
        assert(utfc.open("windows-1255",skip));
        assert(utfc.convert(shalom_pease_utf8.c_str(),shalom_pease_utf8.c_str()+shalom_pease_utf8.size()) == shalom_1255+"  world");
    }

    std::cerr << "From Ok" << std::endl;

    {
        uconv_to_utf<char> utfc;
        uconv_to_utf<wchar_t> wcharc;
        assert(utfc.open("windows-1255",stop));
        assert(wcharc.open("windows-1255",stop));
        assert(utfc.convert(shalom_1255.c_str(),shalom_1255.c_str()+shalom_1255.size())==shalom_utf8);
        assert(wcharc.convert(shalom_1255.c_str(),shalom_1255.c_str()+shalom_1255.size())==shalom_wchar_t);
    }
    
    std::cerr << "To Ok" << std::endl;
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
