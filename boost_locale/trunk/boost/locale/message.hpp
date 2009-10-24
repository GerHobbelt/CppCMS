#ifndef BOOST_LOCALE_MESSAGE_HPP_INCLUDED
#define BOOST_LOCALE_MESSAGE_HPP_INCLUDED

#include <locale>

namespace boost {
    namespace locale {
        
        template<typename CharType>
        class messages : public std::locale::facet
        {
        public:
            
            typedef CharType char_type;

            static std::locale::id id;

            virtual char_type const *get(int domain_id,char const *id) const = 0;
            virtual char_type const *get(int domain_id,char const *single_id,int n) const = 0;
            virtual int get_domain_id(std::string const &domain) const = 0;

        protected:
            virtual ~messages()
            {
            }

        };

        template<typename CharType>
        static std::locale::id messages::id;
        
        struct message {
        public:
            message(std::string const &id) :
                id_(id),
                n_(0)
            {
            }
            message(std::string const &single,std::string const &plural,int number) :
                id_(single),
                plural_(plural),
                n_(number)
            {
            }
            
            template<typename CharType>
            void write(std::basic_ostream<CharType> &output) const
            {
                std::locale const &loc = output.getloc();
                if(std::has_facet<messages<CharType> >(loc)) {
                    messages<CharType> const &msg = std::use_facet<messages<CharType> >(loc);
                    std::basic_string<CharType> buffer;
                    CharType const *translated = 0;
                    if(plural_.empty())
                        translated=msg.get(id_.c_str(),buffer);
                    else
                        translated=msg.get(id_.c_str(),buffer,n_);
                    if(translated)
                        
                }
            }
        private:
            int n_;
            std::string id_;
            std::string plural_;
        };
    }
}


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

