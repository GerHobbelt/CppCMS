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

        ///
        /// \brief this class represents localization backend that can be used for localizing your application.
        /// 
        /// Backends are usually registered inside localization backends manager and allow transparent support
        /// of different backends, so user can swithc backend by simply linking the application to correct one.
        ///
        /// Backends may support different tuning options, but these are the default options avalible for user
        /// for all of them
        ///
        /// -# locale - the name of the locale in POSIX format like en_US.UTF-8 or en_US
        /// -# encoding  - default character set encoding
        /// -# message_path - path to location of message catalogs vector of strings
        /// -# message_application - the name of applications that use message catalogs (vector of strings)
        /// 
        /// Each backend may be installed with different default priority so when you work with two differnt backends, you
        /// can specify priotiry so this backend will be chosen according to their priority.
        ///
        
        class localization_backend {
            localization_backend(localization_backend const &);
            void operator=(localization_backend const &);
        public:

            localization_backend()
            {
            }

            virtual localization_backend *clone() const = 0;

            ///
            /// Get backend name, for example "icu", "std" or "Qt"
            ///
            virtual std::string name() const = 0;
            ///
            /// Get a priority for the backend 
            ///
            virtual int priority() const = 0;

            ///
            /// Set option for backend, for example "locale" or "encoding"
            ///
            virtual void set_option(std::string const &name,std::string const &value) = 0;

            ///
            /// Clear all options
            ///
            virtual void clear_options();

            ///
            /// Create a facet for category \a category and character type \a type 
            ///
            virtual std::locale::facet *create(locale_category_type category,character_facet_type type = nochar_facet) = 0;

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

            bool register_backend(std::auto_ptr<localization_backend> backend);
            void unregister_backend(std::string const &s);

            std::multimap<int,std::string> all_backends() const;

            std::auto_ptr<localization_backend> get_backend(locale_category_type category) const;
            std::auto_ptr<localization_backend> get_backend(std::string const &name) const;

            void set_default(std::string const &name,locale_category_type category = all_categories);

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

