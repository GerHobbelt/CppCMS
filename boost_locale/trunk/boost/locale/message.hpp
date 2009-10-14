#ifndef BOOST_LOCALE_MESSAGE_HPP_INCLUDED
#define BOOST_LOCALE_MESSAGE_HPP_INCLUDED

#include <locale>

namespace boost {
    namespace locale {

        class messages_base {
        public:
            static void set_domain_id(std::ios_base &ios,int id);
            static int  get_domain_id(std::ios_base &ios);
        };

        class domain {
        public:
            domain(std::string const &str) : domain_(str) {}

            template<typename CharType>
            friend std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out,domain const &d)
            {
                int id=std::use_facet<messages<CharType> >(out.getloc()).get_domain_id(d.domain_);
                messages_base::set_domain_id(out,id);
            }

        private:
            std::string domain_;
        };

        template<typename CharType>
        class messages : public std::locale::facet
        {
        public:
            
            typedef std::basic_string<CharType> string_type;
            typedef CharType char_type;

            static std::locale::id id;

            char_type const *get(std::ios_base &ios,char const *id) const
            {
                return do_get(get_domain_id(ios),id);
            }

            char_type const *get(std::ios_base &ios,char const *single_id,char const *plural,int n) const
            {
                return do_get(get_domain_id(ios),single_id,plural,n);
            }

            int get_domain_id(std::string const &domain) const
            {
                return do_get_domain_id(std::string const &domain);
            }

        protected:

            virtual ~messages() {}


            virtual char_type const *get(int domain_id,char_type const *id) const = 0;
            virtual char_type const *get(int domain_id,char_type const *single_id,char_type const *plural,int n) const = 0;
            virtual int get_domain_id(std::string const &domain) const = 0;
        };

        class tr {
        public:
            tr(std::string const &single) :
                single_(single.c_str()),
                plural_(0),
                n_(0),
            {
            }

            tr(std::string const &single,std::string const &plural,int n) :
                signle_(single.c_str()),
                plural_(plural.c_str()),
                n_(n)
            {
            }

            tr(char const *single) :
                single_(single),
                plural_(0),
                n_(0)
            {
            }

            tr(char const *single,char const *plural,int n) :
                signle_(single),
                plural_(plural),
                n_(n)
            {
            }

            template<typename CharType>
            friend std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out,tr const &t)
            {
                typedef messages<CharType> msg_t;
                typedef std::ctype<CharType> ctype_t;

                if(!std::has_facet<msg_t>(out.getloc())) {
                    char const *msg = plural_ ? ( n == 1 ? single_ : plural_ ) : single_;
                    switch(CharType) {
                    case char:
                        out<<msg;
                        break;
                    default:
                        {
                            ctype_t const &c=std::use_facet<ctype_t>(out.getloc());
                            while(*msg)
                                out<<c.widen(*msg++);
                        }
                    }

                }
                else {
                    msg_t const &msg = std::use_facet<msg_t>(out.getloc())
                    
                    if(plural_)
                        out << message.gt(single_,plural_,n_);
                    else
                        out << message.gt(single_);
                }

                return out;
            }

        private:
            char const *single_,
            char const *plural_,
            int n;
        };

        template<typename CharType>
        static std::locale::id messages::id;
    }

}


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

