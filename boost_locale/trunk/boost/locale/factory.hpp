namespace boost {
    namespace locale {
        namespace encoding {
            typedef enum {
                enc_char        = 1 << 0,
                enc_wchar_t     = 1 << 1,
                enc_char16_t    = 1 << 2,
                enc_char32_t    = 1 << 3
                enc_all         = 0xFFFFFFFFu
            } encoding_type;
        }

        namespace category {
            typedef enum {
                lc_ctype      = 1 << 0,
                lc_numeric    = 1 << 1,
                lc_time       = 1 << 2,
                lc_collate    = 1 << 3,
                lc_monetary   = 1 << 4,
                lc_messages   = 1 << 5,            
                lc_all        = 0xFFFFFFFFu,
            } category_type;
        }

        class factory {
        public:
            factory();
            factory(std::string posix_locale,unsigned categories = category::lc_all);
            void language(std::string str,unsigned categories = category::lc_all);
            void region(std::string str,unsigned categories = category::lc_all);
            void charset(std::string str,unsigned categories = category::lc_all);
            void variant(std::string str,unsigned categories = category::lc_all);
            void add_messages_path(std::string path);
            void add_messages_domain(std::string domain);
            void set_messages_default_domain(std::string domain);

            std::locale generate(std::locale const &original,unsigned enc=encoding::enc_all);

            ~factory();
        private:
            factory(factory const &other);
            factory const &operator=(factory const &other);
        };

        std::locale generate_locales(std::locale const &existing,unsigned categories_mask,std::string const &posix_name)
    }

}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
