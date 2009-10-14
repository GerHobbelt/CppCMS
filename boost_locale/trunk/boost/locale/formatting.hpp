namespace boost {
    namespace locale {
        namespace as {
            typedef enum {
                posix           = 0,
                number          = 1,
                scientific      = 2,
                currency        = 3,
                percent         = 4,
                date            = 5
                time            = 6,
                datetime        = 7,
                spellout        = 8,
                ordinal         = 9,
                time_duration   = 10,

                currency_default  = 0 << 5,
                currency_iso      = 1 << 5,
                currency_national = 2 << 5,

                time_default    = 0 << 7,
                time_short    = 1 << 7,
                time_medium    = 2 << 7,
                time_long    = 3 << 7,
                time_full    = 4 << 7,
                
                date_default    = 0 << 10,
                date_short    = 1 << 10,
                date_medium    = 2 << 10,
                date_long    = 3 << 10,
                date_full    = 4 << 10,

            } display_type;
        }

        uint64_t ios_flags(std::ios_base &ios);
        uint64_t ios_flags(std::ios_base &ios,uint64_t new_flags);

        
        template<typename CharType>
        void set_time_format(std::ios_base &,std::basic_string<CharType> const &str);

	template<>
	void set_time_format(std::ios_base &,std::string const &str);
        
	template<>
	void set_time_format(std::ios_base &,std::wstring const &str);
	
	template<>
	void set_time_format(std::ios_base &,std::u16string const &str);
	
	template<>
	void set_time_format(std::ios_base &,std::u32string const &str);


            
            /// Default value output format
            ///
            std::ios_base &posix(std::ios_base &);

            ///
            /// Display value as number in locale format
            ///
            std::ios_base &number(std::ios_base &);

            ///
            /// Display value as number in scientific format
            ///
            std::ios_base &scientific(std::ios_base &);
            
            ///
            /// Display value as currency
            ///
            std::ios_base &currency(std::ios_base &);
            
            ///
            /// Display value in percent format
            ///
            std::ios_base &percent(std::ios_base &);

            ///
            /// Spellout value (three)
            ///
            std::ios_base &spellout(std::ios_base &);

            /// 
            /// Display value as ordinal format (3rd)
            ///
            std::ios_base &ordinal(std::ios_base &);

            ///
            /// Use ISO 3 letters currency identifier (like USD)
            ///
            std::ios_base &iso_currency(std::ios_base &);

            ///
            /// Use national currency identifier like $
            ///
            std::ios_base &national_currency(std::ios_base &);


            typedef enum {
                fmt_default,
                fmt_short,
                fmt_medium,
                fmt_long,
                fmt_full
            } format_type;

            void set_date_format(std::ios_base &ios,format_type f);
            void set_time_format(std::ios_base &ios,format_type f);
            void set_datetime_format(std::ios_base &ios,format_type d,format_type t);

            struct date {
                date(format_type fmt = fmt_default) : fmt_(fmt)
                {
                }
                template<CharType>
                friend std::basic_ostream &operator<<(std::basic_ostream &out,date const &d)
                {
                    set_date_format(out,fmt_);
                }
            private:
                format_type fmt_;
            };
            
            struct time {
                time(format_type fmt = fmt_default) : fmt_(fmt)
                {
                }
                template<CharType>
                friend std::basic_ostream &operator<<(std::basic_ostream &out,time const &d)
                {
                    set_time_format(out,fmt_);
                }
            private:
                format_type fmt_;
            };
            
            struct datetime {
                datetime(format_type d,format_type t) : dfmt_(d), tfmt_(t)
                {
                }
                datetime(format_type fmt = fmt_default) : dfmt_(fmt), tfmt_(fmt)
                {
                }
                template<CharType>
                friend std::basic_ostream &operator<<(std::basic_ostream &out,datetime const &d)
                {
                    set_datetime_format(out,dfmt_,tfmt_);
                }
            private:
                format_type dfmt_;
                format_type tfmt_;
            };
            

        } // as
    } // locale
} // boost
