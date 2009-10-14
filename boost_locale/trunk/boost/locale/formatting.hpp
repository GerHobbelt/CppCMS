namespace boost {
    namespace locale {
        namespace as {
            typedef enum {
                posix               = 0,
                number              = 1,
                scientific          = 2,
                currency            = 3,
                percent             = 4,
                permil              = 5,
                date                = 6
                time                = 7,
                datetime            = 8,
                spellout            = 9,
                ordinal             = 10,
                time_duration       = 11,

                display_flags_mask  = 0x1F.

                currency_default    = 0 << 5,
                currency_iso        = 1 << 5,
                currency_national   = 2 << 5,

                currency_flags_mask = 3 << 5

                time_default        = 0 << 7,
                time_short          = 1 << 7,
                time_medium         = 2 << 7,
                time_long           = 3 << 7,
                time_full           = 4 << 7,
                time_flags_mask     = 7 << 7,

                date_default        = 0 << 10,
                date_short          = 1 << 10,
                date_medium         = 2 << 10,
                date_long           = 3 << 10,
                date_full           = 4 << 10,
                date_flags_mask     = 7 << 10,

                datetime_flags_mask = date_flags_mask | time_flags_mask;

            } display_flags_type;
        } // as

        class format_info {
        public:
            format_info();
            format_info(format_info const &other);
            format_info const &operator=(format_info const &);
            ~format_info();
            
            uint64_t flags() const;
            void flags(uint64_t new_flags) const;

            void ftime(std::string const &str) const;
            void ftime(std::wstring const &str) const;
#ifdef BOOST_HAVE_UXXSTRING
            void ftime(std::u16string const &str) const;
            void ftime(std::u32string const &str) const;
#endif
    
        };

    } // locale
} // boost
