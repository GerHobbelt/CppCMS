#ifndef BOOST_LOCALE_GENERATOR_HPP
#define BOOST_LOCALE_GENERATOR_HPP
#include <boost/locale/config.hpp>
#include <string>
#include <locale>
#include <memory>

namespace boost {
    namespace locale {
        typedef enum {
            char_facet      = 1 << 0,
            wchar_t_facet   = 1 << 1,
            char16_t_facet  = 1 << 2,
            char32_t_facet  = 1 << 3,

            all_characters = 0xFFFF
        } character_facet_type;

        typedef enum {
            collation_facet = 1 << 0,
            formatting_facet= 1 << 1,
            message_facet   = 1 << 2,
            codepage_facet=   1 << 3,
            
            all_categories  = 0xFFFFFFFFu
        } locale_category_type;

        class BOOST_LOCALE_DECL generator {
        public:

            generator();
            ~generator();

            void categories(unsigned cats);
            unsigned categories() const;
            
            void characters(unsigned chars);
            unsigned characters() const;

            void octet_encoding(std::string const &encoding);
            std::string octet_encoding() const;

            void add_messages_domain(std::string const &domain);
            void set_default_messages_domain(std::string const &domain);
            void clear_domains();

            void add_messages_path(std::string const &path);
            void clear_paths();

            void clear_cache();

            void preload(std::string const &id);
            void preload(std::string const &id,std::string const &encoding);

            void preload(std::locale const &base,std::string const &id);
            void preload(std::locale const &base,std::string const &id,std::string const &encoding);

            std::locale generate(std::string const &id) const;
            std::locale generate(std::string const &id,std::string const &encoding) const;

            std::locale generate(std::locale const &base,std::string const &id) const;
            std::locale generate(std::locale const &base,std::string const &id,std::string const &encoding) const;

            std::locale get(std::string const &id) const;
            std::locale get(std::string const &id,std::string const &encoding) const;

            std::locale operator()(std::string const &id) const
            {
                return get(id);
            }
            std::locale operator()(std::string const &id,std::string const &encoding) const
            {
                return get(id,encoding);
            }

        private:

            template<typename CharType>
            std::locale generate_for(std::locale const &source) const;
            std::locale complete_generation(std::locale const &source) const;

            generator(generator const &);
            void operator=(generator const &);

            struct data;
            std::auto_ptr<data> d;
        };

    }
}


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 

