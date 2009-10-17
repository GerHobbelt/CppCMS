#ifndef BOOST_LOCALE_FORMATTER_HPP_INCLUDED
#define BOOST_LOCALE_FORMATTER_HPP_INCLUDED

#include <string>
#include <boost/cstdint.hpp>

namespace boost {
    namespace locale {

        template<typename CharType>
        class formatter {
        public:
            typedef CharType char_type;
            typedef std::basic_string<CharType> string_type;

            virtual string_type format(double value,size_t &code_points) const = 0;
            virtual string_type format(int64_t value,size_t &code_points) const = 0;
            virtual string_type format(uint64_t value,size_t &code_points) const = 0;
            virtual string_type format(int32_t value,size_t &code_points) const = 0;
            virtual string_type format(uint32_t value,size_t &code_points) const = 0;

            virtual bool parse(string_type &str,double &value) const = 0;
            virtual bool parse(string_type &str,int64_t &value) const = 0;
            virtual bool parse(string_type &str,uint64_t &value) const = 0;
            virtual bool parse(string_type &str,int32_t &value) const = 0;
            virtual bool parse(string_type &str,uint32_t &value) const = 0;

            virtual ~formatter()
            {
            }
        };

        template<typename CharType>
        class value_format : public std::locale::facet {
        public:
            static std::locale::id id;
            virtual formatter const &get_formatter(std::ios_base &ios) const = 0;  
            virtual ~value_format()
            {
            }
        };

        template<typename CharType>
        std::locale::id value_format::id;


}



#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
