#ifndef BOOST_LOCALE_CATEGORIES_HPP
#define BOOST_LOCALE_CATEGORIES_HPP

namespace boost {
    namespace locale {
        typedef enum {
            char_facet      = 1 << 0,
            wchar_t_facet   = 1 << 1,
            char16_t_facet  = 1 << 2,
            char32_t_facet  = 1 << 3,

            all_charracters = 0xFFFF
        } charrector_facet_type;

        typedef enum {
            collation_facet = 1 << 0,
            formatting_facet= 1 << 1,
            message_facet   = 1 << 2,
            conversion_facet= 1 << 3,
            boundary_facet  = 1 << 4,
            
            all_categories  = 0xFFFFFFFFu
        } locale_category_type;

        class factory {
        public:

            factory();
            ~factory();


            void categories(locale_category_type cats);
            locale_category_type categories() const;
            
            void charrecters(charrector_facet_type chars);
            charrector_facet_type charrecters() const;
            
            void octet_encoding(std::string const encoding);
            std::string octet_encoding() const;

            void add_messages_domain(std::string const &domain);
            void set_default_messages_domain(std::string const &domain);
            void clear_domains();

            void add_messages_path(std::string const &path);
            void clear_paths();


            std::locale generate(std::locale const &base,std::string const &id);
            std::locale generate(std::locale const &base,std::string const &id,std::string const &encoding);

            std::locale get(std::string const &id) const;
            std::locale get(std::string const &id,std::string const &encoding) const;

        private:

            factory(factory const &);
            void operator=(factory const &);

            std::auto_ptr<factory_impl> impl_;
        };

    }
}


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 

