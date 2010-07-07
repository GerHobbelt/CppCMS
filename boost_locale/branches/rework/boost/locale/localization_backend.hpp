//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_LOCALIZATION_BACKEND_HPP
#define BOOST_LOCALE_LOCALIZATION_BACKEND_HPP
#include <boost/locale/config.hpp>
#ifdef BOOST_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4275 4251 4231 4660)
#endif
#include <string>
#include <locale>
#include <boost/shared_ptr.hpp>

namespace boost {
    namespace locale {
        
        class facets_generator {

            facets_generator(facets_generator const &);
            void operator = (facets_generator const &)

        public:
            
            virtual std::locale::facet *create(locale_category_type category,character_facet_type char_type) const 
            {
                return 0;
            }

            facets_generator()
            {
            }

            virtual ~facets_generator()
            {
            }
        };

        class localization_backend {
            localization_backend(localization_backend const &);
            void operator=(localization_backend const &);
        public:
            localization_backend()
            {
            }

            virtual std::string name() const = 0;
            virtual int priority() const = 0;
            virtual localization_backend *clone() const = 0;

            virtual void set_option(std::string const &name,std::string const &value) = 0;
            boost::shared_ptr<facets_generator> get_generator() const = 0;
            virtual ~localization_backend()
            {
            }
        };

        class BOOST_LOCALE_DECL localization_backends_manager {

            localization_backends_manager();
            ~localization_backends_manager();
            localization_backends_manager(localization_backends_manager const &);
            void operator = (localization_backends_manager const &);

        public:
            static localization_backends_manager &instance();

            void register_backend(boost::shared_ptr<localization_backend> backend);
            void unregister_backend(boost::shared_ptr<localization_backend> backend);
            void unregister_backend(std::string const &s);

            std::multimap<int,std::string> all_backends() const;

            void set_default(std::string const &name);
            boost::shared_ptr<localization_backend> get_default() const;


        private:
            std::multimap<int priority,boost::shared_ptr<localization_backend> > backends_;
            boost::shared_ptr<localization_backend> current_backend_;
        };

    }
}








#ifdef BOOST_MSVC
#pragma warning(pop)
#endif


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 

