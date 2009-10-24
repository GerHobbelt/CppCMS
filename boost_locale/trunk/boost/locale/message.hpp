#ifndef BOOST_LOCALE_MESSAGE_HPP_INCLUDED
#define BOOST_LOCALE_MESSAGE_HPP_INCLUDED

#include <locale>

namespace boost {
    namespace locale {
        
        struct message_format_base {
        public:
            typedef enum {
                gnu_mo              = 1 << 0,
                gnu_po              = 1 << 1,
                xliff               = 1 << 2,
                search_system_path  = 1 << 8,
                search_wd           = 1 << 9,

                default_flags       = gnu_mo | search_system_path
            } catalog_flags;
        };

        template<typename CharType>
        class message_format : public std::locale::facet
        {
        public:
            typedef CharType char_type;
            typedef std::basic_string<CharType> string_type;

            static std::locale::id id;

            virtual char_type const *get(int domain_id,char const *id,string_type &buffer) const = 0;
            virtual char_type const *get(int domain_id,char const *single_id.string_type &buffer,int n) const = 0;
            virtual int domain(std::string const &domain) const = 0;

        protected:
            virtual ~message_format()
            {
            }

        };

        template<typename CharType>
        static std::locale::id messages::id;
        

        ///
        /// This class represents a message that can be converted to specific locale message
        ///
        struct message {
        public:

            ///
            /// Create default empty message
            /// 
            message() :
                n_(0),
                c_id_(""),
                c_plural_(0)
            {
            }

            ///
            /// Create a simple message from 0 terminated string. The string should exist
            /// until message is destroyed. Generally useful with static constant strings
            /// 
            message(char const *id) :
                n_(0),
                c_id_(id),
                c_plural_(0)
            {
            }

            ///
            /// Create a simple plural form message from 0 terminated strings. The strings should exist
            /// until message is destroyed. Generally useful with static constant strings.
            ///
            /// \a n is the number, \a single and \a plural are single and plural forms of message
            /// 
            message(char const *single,char const *plural,int n) :
                n_(n),
                c_id_(single),
                c_plural_(plural)
            {
            }

            ///
            /// Create a simple message from string.
            ///
            message(std::string const &id) :
                n_(0),
                c_id_(0),
                c_plural_(0),
                id_(id)
            {
            }

            ///
            /// Create a simple plural form message from strings.
            ///
            /// \a n is the number, \a single and \a plural are single and plural forms of message
            /// 
            message(std::string const &single,std::string const &plural,int number) :
                n_(number),
                c_id_(0),
                c_plural_(0),
                id_(single),
                plural_(plural)
            {
            }

            ///
            /// Translate message to the string in default global locale, using default domain
            ///
            template<typename CharType>
            std::basic_string<CharType> str() const
            {
                std::locale loc;
                return str(loc,0);
            }
            
            ///
            /// Translate message to string in the locale \a locale, using default domain
            ///
            template<typename CharType>
            std::basic_string<CharType> str(std::locale const &locale) const
            {
                return str(locale,0);
            }
           
            ///
            /// Translate message to string using locale \a locale and message domain  \ a domain_id
            /// 
            template<typename CharType>
            std::basic_string<CharType> str(std::locale const &locale,std::string domain_id) const
            {
                int id=0;
                if(std::has_facet<messages<CharType> >(locale))
                    id=std::use_facet<messages<CharType> >(locale).domain(domain_id);
                return str(locale,id);
            }

            
            ///
            /// Translate message to string using locale \a locale and message domain index  \ a id
            /// 
            template<typename CharType>
            std::basic_string<CharType> str(std::locale const &locale,int id) const
            {
                std::basic_string<CharType> buffer;                
                CharType const *ptr = write(loc,id,buffer);
                if(ptr == buffer.c_str())
                    return buffer;
                else
                    buffer = ptr;
                return buffer;
            }


            ///
            /// Translate message and write to stream \a out, using imbued locale and domain set to the 
            /// stream
            ///
            template<typename CharType>
            void write(std::basic_ostream<CharType> &out)
            {
                std::locale const &loc = out.getloc();
                int id = ext_value(out,flags::domain_id);
                std::basic_string<CharType> buffer;
                out << write(loc,id,buffer);
            }

        private:
            
            template<typename CharType>
            CharType const *write(std::locale const &loc,int id,std::basic_string<CharType> &buffer) const
            {
                CharType const *translated = 0;

                char const *id = c_id_ ? c_id_ : id_.c_str();
                char const *plural = c_plural_ ? c_plural_ : (plural_.empty() ? 0 : plural_.c_str());
                
                if(std::has_facet<messages<CharType> >(loc)) {
                    messages<CharType> const &msg = std::use_facet<messages<CharType> >(loc);
                    
                    if(!plural_) {
                        translated = msg.get(domain_id,id,buffer);
                    }
                    else {
                        translated = msg.get(domain_id,id,buffer,n_);
                    }
                }

                if(!translated) {
                    char const *msg = plural ? ( n_ == 1 ? cut_comment(id) : plural) : cut_comment(id);

                    /// FIXME -- need convert codepage from UTF-8 instead

                    std::ctype<CharType> const &ct = std::use_facet<std::ctype<CharType> >(loc);
                    while(*msg)
                        buffer.append(ct.widen(*msg++));

                    translated = buffer.c_str();
                }
                return translated;
            }

            char const *cut_comment(char const *id)
            {
                static const char key = '#';
                if(*id == key) {
                    if(id[1] == key)
                        return id+1;
                    char c;
                    while((c=*id)!=0 && c!=key)
                        id++;
                    if(*id==key)
                        id++;
                }
                return id;
            }

            /// members

            int n_;
            char const *c_id_;
            char const *c_plural_;
            std::string id_;
            std::string plural_;
        };

        ///
        /// Translate message \a msg and write it to stream
        ///
        template<typename CharType>
        std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out,message const &msg)
        {
            msg.write(out);
            return out;
        }


        inline message _(char const *msg)
        {
            return message(msg);
        }
        inline message _(char const *single,char const *plural,int n)
        {
            return message(single,plural,n);
        }
    }
}


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

