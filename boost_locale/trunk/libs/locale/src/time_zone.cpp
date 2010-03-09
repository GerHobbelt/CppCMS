//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_LOCALE_SOURCE
#include <boost/locale/time_zone.hpp>
#include <boost/locale/info.hpp>
#include <unicode/timezone.h>
#include <unicode/strenum.h>
#include "uconv.hpp"
#include "time_zone_impl.hpp"
#include "info_impl.hpp"

namespace boost{
    namespace locale {

        void time_zone::global(time_zone const &zone)
        {
            icu::TimeZone::adoptDefault(zone.impl()->icu_tz()->clone());
        }

        time_zone::time_zone() : impl_(new time_zone_impl())
        {

        }
        time_zone::~time_zone()
        {
        }
        time_zone::time_zone(time_zone const &other) :
            impl_(other.impl_->clone())
        {
        }
        time_zone const &time_zone::operator=(time_zone const &other) 
        {
            if(&other!=this) {
                impl_.reset(other.impl_->clone());
            }
            return *this;
        }

        time_zone::time_zone(std::string const &id) : impl_(new time_zone_impl(id))
        {
        }
        std::string time_zone::id() const
        {
            return impl_->id();
        }
        double time_zone::offset_from_gmt(double time,bool islt) const
        {
            return impl_->offset(time,islt);
        }

        bool time_zone::operator==(time_zone const &other) const
        {
            return *impl_==*other.impl_;
        }
       
        std::set<std::string> time_zone::all_zones()
        {
            std::auto_ptr<icu::StringEnumeration> all(icu::TimeZone::createEnumeration());
            std::set<std::string> result;
            UErrorCode err=U_ZERO_ERROR;
            char const *str;
            while((str=all->next(0,err))!=0) {
                err=U_ZERO_ERROR;
                result.insert(str);
            }
            return result;
        }

        namespace {
            template<typename CharType>
            void write_string(std::basic_ostream<CharType> &out,icu::UnicodeString const &str)
            {
                impl::icu_std_converter<CharType> cvt(std::use_facet<info>(out.getloc()).encoding());
                out << cvt.std(str);
            }
        }
        
        template<>
        BOOST_LOCALE_DECL std::basic_ostream<char> &operator<<(std::basic_ostream<char> &out,time_zone const &tz)
        {
            write_string(out,tz.impl()->name(out.getloc()));
            return out;
        }
    
        #ifndef BOOST_NO_STD_WSTRING
        template<>
        BOOST_LOCALE_DECL std::basic_ostream<wchar_t> &operator<<(std::basic_ostream<wchar_t> &out,time_zone const &tz)
        {
            write_string(out,tz.impl()->name(out.getloc()));
            return out;
        }
        #endif
    
        
        #ifdef BOOST_HAS_CHAR16_T
        template<>
        BOOST_LOCALE_DECL std::basic_ostream<char16_t> &operator<<(std::basic_ostream<char16_t> &out,time_zone const &tz)
        {
            write_string(out,tz.impl()->name(out.getloc()));
            return out;
        }
        #endif
    
        #ifdef BOOST_HAS_CHAR32_T
        template<>
        BOOST_LOCALE_DECL std::basic_ostream<char32_t> &operator<<(std::basic_ostream<char32_t> &out,time_zone const &tz)
        {
            write_string(out,tz.impl()->name(out.getloc()));
            return out;
        }
        #endif

    }
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
