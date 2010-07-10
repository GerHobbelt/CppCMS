//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_GENERATOR_HPP
#define BOOST_LOCALE_GENERATOR_HPP
#include <boost/locale/config.hpp>
#ifdef BOOST_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4275 4251 4231 4660)
#endif
#include <string>
#include <locale>
#include <memory>

namespace boost {
    ///
    /// \brief This is the main namespace that encloses all localization classes 
    ///
    namespace locale {

        ///
        /// a enum type that specifies the character type that locales can be generated for
        /// 
        typedef enum {
            nochar_facet    = 0,        ///< Unspecified character category for character independed facets
            char_facet      = 1 << 0,   ///< 8-bit character facets
            wchar_t_facet   = 1 << 1,   ///< wide character facets
            char16_t_facet  = 1 << 2,   ///< C++0x char16_t facets
            char32_t_facet  = 1 << 3,   ///< C++0x char32_t facets

            character_first_facet = char_facet,
            character_last_facet = char32_t_facet,

            all_characters = 0xFFFF     ///< Special mask -- generate all
        } character_facet_type;

        ///
        /// a special enum used for more fine grained generation of facets
        ///
        typedef enum {
            convert_facet   = 1 << 0,   ///< Generate convertsion facets
            collation_facet = 1 << 1,   ///< Generate collation facets
            formatting_facet= 1 << 2,   ///< Generate numbers, currency, date-time formatting facets
            parsing_facet   = 1 << 3,   ///< Generate numbers, currency, date-time formatting facets
            message_facet   = 1 << 4,   ///< Generate message facets
            codepage_facet  = 1 << 5,   ///< Generate codepage conversion facets (derived from std::codecvt)
            boundary_facet  = 1 << 6,   ///< Generate boundary analysis facet
            
            per_character_facet_first = convert_facet,
            per_character_facet_last = boundary_facet,

            calendar_facet  = 1 << 16,   ///< Generate boundary analysis facet
            information_facet  
                            = 1 << 17,   ///< Generate general locale information facet

            non_character_facet_first = calendar_facet,
            non_character_facet_last = information_facet
            
            all_categories  = 0xFFFFFFFFu   ///< Generate all of them
        } locale_category_type;

        ///
        /// \brief the major class used for locale generation
        ///
        /// This class is used for specification of all parameters required for locale generation and
        /// caching. This class const member functions are thread safe if locale class implementation is thread safe.
        ///

        class BOOST_LOCALE_DECL generator {
        public:

            generator();
            ~generator();

            ///
            /// Set types of facets that should be generated, default all
            ///
            void categories(unsigned cats);
            ///
            /// Get types of facets that should be generated, default all
            ///
            unsigned categories() const;
            
            ///
            /// Set the characters type for which the facets should be generated, default all supported
            ///
            void characters(unsigned chars);
            ///
            /// Get the characters type for which the facets should be generated, default all supported
            ///
            unsigned characters() const;

            ///
            /// Add a new domain of messages that would be generated. It should be set in order to enable
            /// messages support.
            ///
            void add_messages_domain(std::string const &domain);
            ///
            /// Set default message domain. If this member was not called, the first added messages domain is used.
            /// If the domain \a domain is not added yet it is added.
            ///
            void set_default_messages_domain(std::string const &domain);

            ///
            /// Remove all added domains from the list
            ///
            void clear_domains();

            ///
            /// Add a search path where dictionaries are looked in.
            /// 
            void add_messages_path(std::string const &path);

            #if defined(BOOST_WINDOWS)
            ///
            /// Add a "wide" search path where dictionaries are looked in. 
            /// This function is Win32 specific only as windows "ANSI" API does not support full
            /// range of paths as on any other operating systems
            /// 
            void add_messages_path(std::wstring const &path);
            #endif

            ///
            /// Remove all added paths
            ///
            void clear_paths();

            ///
            /// Remove all cached locales
            ///
            void clear_cache();

            ///
            /// Turn locale caching ON
            ///
            void locale_cache_enabled(bool on);

            ///
            /// Get locale cache option
            ///
            bool locale_cache_enabled() const;

            ///
            /// Generate a locale with id \a id
            ///
            std::locale generate(std::string const &id) const;
            ///
            /// Generate a locale with id \a id, use \a base as a locale for which all facets are added,
            /// instead of std::locale::classic() one
            ///
            std::locale generate(std::locale const &base,std::string const &id) const;
            ///
            /// Shortcut to generate(id)
            ///
            std::locale operator()(std::string const &id) const
            {
                return generate(id);
            }
            
            ///
            /// Set backend specific option
            ///
            void set_option(std::string const &name,std::string const &value);

            ///
            /// Clear backend specific options
            ///
            void clear_options();

        private:

            generator(generator const &);
            void operator=(generator const &);

            struct data;
            std::auto_ptr<data> d;
        };

    }
}
#ifdef BOOST_MSVC
#pragma warning(pop)
#endif


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 

