//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_LOCALE_SOURCE
#include <boost/locale/localization_backend.hpp>
#include <boost/locale/gnu_gettext.hpp>
#include "all_generator.hpp"
#include "cdata.hpp"
#include "icu_backend.hpp"

#include <unicode/ucnv.h>

namespace boost {
namespace locale {
namespace impl_icu { 
    class icu_localization_backend : public localization_backend {
    public:
        icu_localization_backend() : 
            invalid_(true)
        {
        }
        icu_localization_backend(icu_localization_backend const &other) : 
            paths_(other.paths_),
            domains_(other.domains_),
            locale_id_(other.locale_id_),
            invalid_(true)
        {
        }
        virtual icu_localization_backend *clone() const
        {
            return new icu_localization_backend(*this);
        }

        void set_option(std::string const &name,std::string const &value) 
        {
            invalid_ = true;
            if(name=="locale")
                locale_id_ = value;
            else if(name=="message_path")
                paths_.push_back(value);
            else if(name=="message_application")
                domains_.push_back(value);

        }
        void clear_options()
        {
            invalid_ = true;
            locale_id_.clear();
            paths_.clear();
            domains_.clear();
        }

        std::string encoding_from_locale(std::string const &str)
        {
            std::string lenc;
            size_t start = str.find('.');
            if(start!=std::string::npos) {
                start++;
                size_t end = str.find('@',start);
                lenc=str.substr(start,end-start);
            }
            if(lenc.empty()) {
                lenc=ucnv_getDefaultName();
            }
            if(ucnv_compareNames(lenc.c_str(),"UTF8")==0) {
                lenc="UTF-8";
            }
            return lenc;
        }

        void prepare_data()
        {
            if(!invalid_)
                return;
            invalid_ = false;
            data_.locale = icu::Locale::createCanonical(locale_id_.c_str());
            data_.encoding = encoding_from_locale(locale_id_);
            data_.utf8 = data_.encoding == "UTF-8";
            language_ = data_.locale.getLanguage();
            country_ = data_.locale.getCountry();
            variant_ = data_.locale.getVariant();
        }
        
        virtual std::locale install(std::locale const &base,
                                    locale_category_type category,
                                    character_facet_type type = nochar_facet)
        {
            prepare_data();

            switch(category) {
            case convert_facet:
                return create_convert(base,data_,type);
            case collation_facet:
                return create_collate(base,data_,type);
            case formatting_facet:
                return create_formatting(base,data_,type);
            case parsing_facet:
                return create_parsing(base,data_,type);
            case codepage_facet:
                return create_codecvt(base,data_.encoding,type);
            case message_facet:
                {
                    gnu_gettext::messages_info minf;
                    minf.language = language_;
                    minf.country = country_;
                    minf.variant = variant_;
                    minf.encoding = data_.encoding;
                    minf.domains = domains_;
                    minf.paths = paths_;
                    switch(type) {
                    case char_facet:
                        return std::locale(base,gnu_gettext::create_messages_facet<char>(minf));
                    #ifndef BOOST_NO_STD_WSTRING
                    case wchar_t_facet:
                        return std::locale(base,gnu_gettext::create_messages_facet<wchar_t>(minf));
                    #endif
                    #ifdef BOOST_HAS_CHAR16_T
                    case char16_t_facet:
                        return std::locale(base,gnu_gettext::create_messages_facet<char16_t>(minf));
                    #endif
                    #ifdef BOOST_HAS_CHAR32_T
                    case char32_t_facet:
                        return std::locale(base,gnu_gettext::create_messages_facet<char32_t>(minf));
                    #endif
                    default:
                        return base;
                    }
                }
            case boundary_facet:
                return create_boundary(base,data_,type); 
            case calendar_facet:
                return create_calendar(base,data_);
            case information_facet:
                return create_info(base,data_);
            default:
                return base;
            }
        }

    private:

        std::vector<std::string> paths_;
        std::vector<std::string> domains_;
        std::string locale_id_;

        cdata data_;
        std::string language_;
        std::string country_;
        std::string variant_;
        bool invalid_;
    };
    
    localization_backend *create_localization_backend()
    {
        return new icu_localization_backend();
    }

}  // impl icu
}  // locale
}  // boost
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
