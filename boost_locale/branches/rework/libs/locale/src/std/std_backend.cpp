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
#include "find_locale_name.hpp"
#include "locale_data.hpp"
#include "std_backend.hpp"

#include <iostream>

namespace boost {
namespace locale {
namespace impl_std { 
    
    class std_localization_backend : public localization_backend {
    public:
        std_localization_backend() : 
            invalid_(true)
        {
        }
        std_localization_backend(std_localization_backend const &other) : 
            paths_(other.paths_),
            domains_(other.domains_),
            locale_id_(other.locale_id_),
            invalid_(true)
        {
        }
        virtual std_localization_backend *clone() const
        {
            return new std_localization_backend(*this);
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

        void prepare_data()
        {
            if(!invalid_)
                return;
            invalid_ = false;
            std::string lid=locale_id_;
            if(lid.empty()) {
                try {
                    std::locale l("");
                    if(l.name()!="*" && l.name()!="POSIX" && l.name()!="C")
                        lid=l.name();
                    else
                        lid="C";
                }
                catch(std::exception const &e) {
                    lid="C";
                }
            }
            data_.parse(lid);
            locale_name::subst_type tmp = locale_name::find(lid);
            name_ = tmp.first;
            utf_mode_ = tmp.second;
        }
        
        virtual std::locale install(std::locale const &base,
                                    locale_category_type category,
                                    character_facet_type type = nochar_facet)
        {
            prepare_data();

            switch(category) {
            case convert_facet:
                return create_convert(base,name_,type,utf_mode_);
            case collation_facet:
                return create_collate(base,name_,type,utf_mode_);
            case formatting_facet:
                return create_formatting(base,name_,type,utf_mode_);
            case parsing_facet:
                return create_parsing(base,name_,type,utf_mode_);
            case codepage_facet:
                return create_codecvt(base,name_,type,utf_mode_);
            case message_facet:
                {
                    gnu_gettext::messages_info minf;
                    minf.language = data_.language;
                    minf.country = data_.country;
                    minf.variant = data_.variant;
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
            case information_facet:
                return create_info(base,name_);
            default:
                return base;
            }
        }

    private:

        std::vector<std::string> paths_;
        std::vector<std::string> domains_;
        std::string locale_id_;

        locale_data data_;
        std::string name_;
        utf8_support utf_mode_;
        bool invalid_;
    };
    
    localization_backend *create_localization_backend()
    {
        return new std_localization_backend();
    }

}  // impl icu
}  // locale
}  // boost
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
