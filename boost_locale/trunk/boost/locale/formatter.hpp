#ifndef BOOST_LOCALE_FORMATTER_HPP_INCLUDED
#define BOOST_LOCALE_FORMATTER_HPP_INCLUDED

namespace boost {
    namespace locale {
        template<typename CharType>
        class formatter {
        public:
            typedef CharType char_type;
            typedef std::basic_string<CharType> string_type;

            virtual string_type format(double value) const = 0;
            virtual string_type format(int64_t value) const = 0;
            virtual string_type format(int32_t value) const = 0;

            virtual bool parse(string_type &str,double &value) const = 0;
            virtual bool parse(string_type &str,int64_t &value) const = 0;
            virtual bool parse(string_type &str,int32_t &value) const = 0;

            virtual formatter *clone() const = 0;

            virtual ~formatter()
            {
            }
        };


    template<typename CharType>
    class formatting : public std::locale::facet {
    public:
        std::auto_ptr<formatter<CharType >> generate();
    }
    }
}



#endif
