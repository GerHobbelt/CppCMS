#ifndef BOOST_LOCALE_HPP_INCLUDED
#define BOOST_LOCALE_HPP_INCLUDED

#include <locale>
#include <string>

namespace boost {
    namespace locale {
        class numeric_base {
        public:
            typedef enum {
                simple,
                normal,
                scientific,
                percent,
                currency_national,
                currency_iso,
                spellout,
                ordinal,
                numbering_system
            } formatting_type;

        };

        template<typname CharType>
        class num_format : 
            public std::num_put<CharType>,
            public numeric_base
        {
        public:
            typedef std::basic_string<CharType> string_type;
            typedef CharType char_type;
            typedef std::num_put<CharType>::iter_type iter_type;

            string_type format(formatting_type how,std::ios_base &ios,char_type fill,bool value) const
            {
                do_format(how,ios,fill,value);
            }
            string_type format(formatting_type how,std::ios_base &ios,char_type fill,long value) const 
            {
                do_format(how,ios,fill,value);
            }
            string_type format(formatting_type how,std::ios_base &ios,char_type fill,unsigned long value)  const
            {
                do_format(how,ios,fill,value);
            }
            string_type format(formatting_type how,std::ios_base &ios,char_type fill,long long value) const
            {
                do_format(how,ios,fill,value);
            }
            string_type format(formatting_type how,std::ios_base &ios,char_type fill,unsigned long long value) const
            {
                do_format(how,ios,fill,value);
            }
            string_type format(formatting_type how,std::ios_base &ios,char_type fill,double value) const
            {
                do_format(how,ios,fill,value);
            }
            string_type format(formatting_type how,std::ios_base &ios,char_type fill,long double value) const
            {
                do_format(how,ios,fill,value);
            }
        protected:
            virtual string_type format(formatting_type how,std::ios_base &ios,bool value) const = 0;
            virtual string_type format(formatting_type how,std::ios_base &ios,long value) const = 0;
            virtual string_type format(formatting_type how,std::ios_base &ios,unsigned long value) const = 0;
            virtual string_type format(formatting_type how,std::ios_base &ios,long long value) const = 0;
            virtual string_type format(formatting_type how,std::ios_base &ios,unsigned long long value) const = 0;
            virtual string_type format(formatting_type how,std::ios_base &ios,double value) const = 0;
            virtual string_type format(formatting_type how,std::ios_base &ios,long double value) const = 0;
            virtual iter_type put(iter_type out,ios_base& ios,char_type fill, bool val) const
            {
                string_type tmp=format()
            }
        private: 
            iter_type put(iter_type out,ios_base &ios,char_type fill,string_type const &str)
            {
                
            }
            formatting_type from_flags(std::ios_base &ios)
            {
                if(ios.fmtflags() & std::ios_base::basefield 
            }

            }
        };
    }
}


#endif


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
