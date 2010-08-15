//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_IMPL_STD_LOCALE_DATA_HPP
#define BOOST_LOCALE_IMPL_STD_LOCALE_DATA_HPP

#include <string>

namespace boost {
    namespace locale {
        namespace impl_std {
            
            class locale_data {
            public:
                locale_data() : 
                    language("C"),
                    utf8(false)
                {
                }

                std::string language;
                std::string country;
                std::string variant;
                std::string encoding;
                bool utf8;

                void parse(std::string const &locale_name)
                {
                    language = "C";
                    country.clear();
                    variant.clear();
                    encoding.clear();
                    utf8=false;
                    parse_from_lang(locale_name);
                }

            private:

                void parse_from_lang(std::string const &locale_name) 
                {
                    size_t end = locale_name.find_first_of("-_@.");
                    std::string tmp = locale_name.substr(0,end);
                    if(tmp.empty())
                        return;
                    for(unsigned i=0;i<tmp.size();i++) {
                        if('A' <= tmp[i] && tmp[i]<='Z')
                            tmp[i]=tmp[i]-'A'+'a';
                        else if(tmp[i] < 'a' && 'z' < tmp[i])
                            return;
                    }
                    language = tmp;
                    if(end >= locale_name.size())
                        return;

                    if(locale_name[end] == '-' || locale_name[end]=='_') {
                       parse_from_country(locale_name.substr(end+1));
                    }
                    else if(locale_name[end] == '.') {
                       parse_from_encoding(locale_name.substr(end+1));
                    }
                    else if(locale_name[end] == '@') {
                       parse_from_variant(locale_name.substr(end+1));
                    }
                }

                void parse_from_country(std::string const &locale_name) 
                {
                    size_t end = locale_name.find_first_of("@.");
                    std::string tmp = locale_name.substr(0,end);
                    if(tmp.empty())
                        return;
                    for(unsigned i=0;i<tmp.size();i++) {
                        if('a' <= tmp[i] && tmp[i]<='a')
                            tmp[i]=tmp[i]-'a'+'A';
                        else if(tmp[i] < 'A' && 'Z' < tmp[i])
                            return;
                    }

                    country = tmp;

                    if(end >= locale_name.size())
                        return;
                    else if(locale_name[end] == '.') {
                       parse_from_encoding(locale_name.substr(end+1));
                    }
                    else if(locale_name[end] == '@') {
                       parse_from_variant(locale_name.substr(end+1));
                    }
                }
                
                void parse_from_encoding(std::string const &locale_name) 
                {
                    size_t end = locale_name.find_first_of("@");
                    std::string tmp = locale_name.substr(0,end);
                    if(tmp.empty())
                        return;
                    for(unsigned i=0;i<tmp.size();i++) {
                        if('A' <= tmp[i] && tmp[i]<='Z')
                            tmp[i]=tmp[i]-'A'+'a';
                    }
                    encoding = tmp;
                    utf8 = encoding == "utf8" || encoding == "utf-8";

                    if(end >= locale_name.size())
                        return;

                    if(locale_name[end] == '@') {
                       parse_from_variant(locale_name.substr(end+1));
                    }
                }

                void parse_from_variant(std::string const &locale_name)
                {
                    variant = locale_name;
                    for(unsigned i=0;i<variant.size();i++) {
                        if('A' <= variant[i] && variant[i]<='Z')
                            variant[i]=variant[i]-'A'+'a';
                    }
                }


            };

        } // impl_std
    } // locale 
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
