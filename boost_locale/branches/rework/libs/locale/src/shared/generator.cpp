//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_LOCALE_SOURCE
#include <boost/locale/info.hpp>
#include <boost/locale/generator.hpp>
#include <boost/locale/collator.hpp>
#include <boost/locale/message.hpp>
#include <boost/locale/codepage.hpp>
#include "numeric.hpp"
#include "conversion.hpp"
#include "collator.hpp"
#include "codepage.hpp"
#include "boundary.hpp"

namespace boost {
    namespace locale {
        struct generator::data {
            data() {
                cats = all_categories;
                chars = all_characters;
            }

            typedef std::map<std::string,std::locale> cached_type;
            cached_type cached;

            unsigned cats;
            unsigned chars;

            std::vector<std::string> paths;
            std::vector<std::string> domains;

            std::map<std::string,std::vector<std::string> > options;


        };

        generator::generator() :
            d(new generator::data())
        {
        }
        generator::~generator()
        {
        }

        unsigned generator::categories() const
        {
            return d->cats;
        }
        void generator::categories(unsigned t)
        {
            d->cats=t;
        }

        void generator::characters(unsigned t)
        {
            d->chars=t;
        }
        
        unsigned generator::characters() const
        {
            return d->chars;
        }

        void generator::add_messages_domain(std::string const &domain)
        {
            if(std::find(d->domains.begin(),d->domains.end(),domain) == d->domains.end())
                d->domains.push_back(domain);
        }
        
        void generator::set_default_messages_domain(std::string const &domain)
        {
            std::vector<std::string>::iterator p;
            if((p=std::find(d->domains.begin(),d->domains.end(),domain)) == d->domains.end()) {
                d->domains.erase(p);
            }
            d->domains.insert(d->domains.begin(),domain);
        }

        void generator::clear_domains()
        {
            d->domains.clear();
        }
        void generator::add_messages_path(std::string const &path)
        {
            d->paths.push_back(path);
        }
        #if defined(BOOST_WINDOWS)
        void generator::add_messages_path(std::wstring const &path)
        {
            std::string utf8_path_with_bom = "\xEF\xBB\xBF";
            utf8_path_with_bom.append(conv::from_utf(path,"UTF-8"));
            add_messages_path(utf8_path_with_bom);
        }
        #endif

        void generator::clear_paths()
        {
            d->paths.clear();
        }
        void generator::clear_cache()
        {
            d->cached.clear();
        }

        std::locale generator::generate(std::string const &id) const
        {
            std::locale base=std::locale::classic();

            return generate(base,id);
        }

        std::locale generator::generate(std::locale const &base,std::string const &id) const
        {
            boost::shared_ptr<localization_backend> backend;
            
            if(!d->backend_manager.get()) 
                backend=d->backend_manager.get();
            else
                backend=localization_backend_manager::global().get();

            set_all_options(backend,id);

            std::locale result = base;
            unsigned facets = d->cats;
            unsigned chars = d->chars;

            for(unsigned facet = per_character_facet_last; facet <= per_character_facet_last && facet!=0; facet <<=1) {
                if(!(facets & facet))
                    continue;
                for(unsigned ch = character_first_facet ; ch<=character_last_facet;ch <<=1) {
                    if(!(ch & chars))
                        continue;
                    std::locale::facet *new_facet = backend->create(facet,ch);
                    if(new_facet)
                        result = std::locale(result,new_facet);
                }
            }
            for(unsigned facet = non_character_facet_first; facet <= non_character_facet_last && facet!=0; facet <<=1) {
                if(!(facets & facet))
                    continue;
                std::locale::facet *new_facet = backend->create(facet);
                if(new_facet)
                    result = std::locale(result,new_facet);
            }
            return result;
        }
        
        void generator::set_all_options(boost::shared_ptr<localization_backend> backend,std::string const &id)
        {
            backend->set_option("locale",id);
            for(unsigned i=0;i<d->domains.size();i++)
                backend->set_option("message_application",d->domains[i]);
            for(unsigned i=0;i<d->paths.size();i++)
                backend->set_option("message_path",d->paths[i]);
        }
        
        namespace details {
            struct initializer_class {
                template<typename CharType>
                static void preload()
                {
                    std::locale l;
                    std::has_facet<info>(l);
                    std::has_facet<num_format<CharType> >(l);
                    std::has_facet<num_parse<CharType> >(l);
                    std::has_facet<collator<CharType> >(l);
                    std::has_facet<std::codecvt<CharType,char,mbstate_t> >(l);
                    std::has_facet<message_format<CharType> >(l);
                }
                initializer_class()
                {
                    preload<char>();
                    #ifndef BOOST_NO_STD_WSTRING
                    preload<wchar_t>();
                    #endif
                    #ifdef BOOST_HAS_CHAR16_T
                    preload<char16_t>();
                    #endif
                    #ifdef BOOST_HAS_CHAR32_T
                    preload<char32_t>();
                    #endif
                }
            } the_initializer;
        } // details

    } // locale
} // boost
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
