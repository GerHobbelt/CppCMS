#ifndef BOOST_LOCALE_FORMAT_HPP_INCLUDED
#define BOOST_LOCALE_FORMAT_HPP_INCLUDED

namespace boost {
    namespace locale {
        namespace details {
            template<typename CharType>
            struct formattible {
                void const *pointer;
                void (*write)(std::basic_ostream<CharType> &output,void const *ptr);

               template<typename Type>
                formattible(Type const &value)
                {
                    pointer = reinterpret_cast<void const *>(&value);
                    write = &do_write<Type>;
                }
            private:
                template<typename Type>
                static void do_write(std::basic_ostream<CharType> &output,void const *ptr)
                {
                    output << *reinterpret_cast<Type const *>(ptr);
                }
           };
        }

        template<typename CharType>
        class basic_format {
            basic_format(std::basic_string<CharType> format_string) : 
                format_(format_string),
                translate_(false)
            {
            }
            basic_format(details::translateable const &trans) : 
                format_(tans.get_string()),
                translate_(true)
            {
            }
            
        private:
        };

    }
}


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

