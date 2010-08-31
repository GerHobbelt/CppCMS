//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_IMPL_STD_FIND_LOCALE_NAME_HPP
#define BOOST_LOCALE_IMPL_STD_FIND_LOCALE_NAME_HPP

#include "all_generator.hpp"
#include "../util/locale_data.hpp"
#include "windows_locale_name.hpp"
namespace boost {
    namespace locale {
        namespace impl_std {
            class locale_name{
            public:
                typedef std::pair<std::string,utf8_support> subst_type;

                static subst_type find(std::string const &name)
                {
                    util::locale_data d;
                    d.parse(name);
                    substs_list_type list = get_substitutions(name,d);
                    for(unsigned i=0;i<list.size();i++) 
                        if(loadable(list[i].first))
                            return list[i];
                    #ifdef BOOST_WINDOWS
                    list = get_substitutions(name,to_windows_name(d));
                    for(unsigned i=0;i<list.size();i++) 
                        if(loadable(list[i].first))
                            return list[i];
                    #endif
                    return subst_type("C",d.utf8 ? utf8_from_wide : utf8_none);
                }
            private:

                static const int use_country = 1;
                static const int use_variant = 2;
                static const int use_encoding = 4;
                
                static util::locale_data to_windows_name(util::locale_data const &din)
                {
                    util::locale_data d = din;
                    d.variant.clear();
                    if(d.encoding.compare(0,2,"cp") == 0)
                        d.encoding=d.encoding.substr(2);
                    else if(d.encoding.compare(0,8,"windows-") == 0)
                        d.encoding=d.encoding.substr(8);
                    else if(d.encoding.compare(0,7,"windows") == 0)
                        d.encoding=d.encoding.substr(7);
                    d.language = get_windows_language_by_code(d.language); 
                    d.country = get_windows_country_by_code(d.country);
                    return d;
                }

                static std::string create_name(util::locale_data const &d,int flags)
                {
                    std::string res;
                    res+=d.language;
                    if((flags & use_country) && !d.country.empty())
                        res+="_" + d.country;
                    if((flags & use_encoding) && !d.encoding.empty())
                        res+="." + d.encoding;
                    if((flags & use_variant) && !d.variant.empty())
                        res+="@" + d.variant; 
                    return res;
                }

                typedef std::vector<std::pair<std::string,utf8_support> > substs_list_type;

                static void add(substs_list_type &res,std::string const &name,utf8_support s)
                {
                    for(unsigned i=0;i<res.size();i++)
                        if(res[i].first==name)
                            return;
                    res.push_back(std::make_pair(name,s));
                }

                static substs_list_type get_substitutions(std::string const &name,util::locale_data const &d)
                {
                    #ifndef BOOST_NO_STD_WSTRING
                    static const bool has_wide = true;
                    const utf8_support utf_flag = d.utf8 ? utf8_native_with_wide : utf8_none;
                    #else
                    static const bool has_wide = false;
                    const utf8_support utf_flag = d.utf8 ? utf8_native : utf8_none;
                    #endif
                    substs_list_type res;

                    add(res,name,utf_flag);
                    add(res,create_name(d,use_country | use_encoding | use_variant),utf_flag);
                    if(has_wide && d.utf8)
                        add(res,create_name(d,use_country | use_variant),utf8_from_wide);

                    add(res,create_name(d,use_country | use_encoding ),utf8_from_wide);
                    if(has_wide && d.utf8)
                        add(res,create_name(d,use_country),utf8_from_wide);
   
                    add(res,create_name(d,use_encoding | use_variant),utf_flag);
                    if(has_wide && d.utf8)
                        add(res,create_name(d,use_variant),utf8_from_wide);
                    return res;
                }
                
                static bool loadable(std::string const &l)
                {
                    try {
                        std::locale tmp(l.c_str());
                        return true;
                    }
                    catch(std::exception const &e) {
                        return false;
                    }
                }
                
            }; // locale_name_finder
        } // impl_std
    } // locale 
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
