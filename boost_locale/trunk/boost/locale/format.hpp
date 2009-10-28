#ifndef BOOST_LOCALE_FORMAT_HPP_INCLUDED
#define BOOST_LOCALE_FORMAT_HPP_INCLUDED

#include <boost/locale/config.hpp>
#include <boost/locale/message.hpp>

#include <sstream>

namespace boost {
    namespace locale {
        namespace details {

            template<typename CharType>
            struct formattible {
                typedef std::basic_ostream<CharType> stream_type;
                typedef void (*writer_type)(stream_type &output,void const *ptr);

                formattible() :
                    pointer_(0),
                    writer_(&formattible::void_write)
                {
                }
                
                formattible(formattible const &other) :
                    pointer_(other.pointer_),
                    writer_(other.writer_)
                {
                }

                formattible const &operator=(formattible const &other)
                {
                    if(this != &other) {
                        pointer_=other.pointer_;
                        writer_=other.writer_;
                    }
                    return *this;
                }
                
                template<typename Type>
                formattible(Type const &value)
                {
                    pointer_ = reinterpret_cast<void const *>(&value);
                    writer_ = &write<Type>;
                }

                template<typename Type>
                formattible const &operator=(Type const &other)
                {
                    *this = formattible(other);
                    return *this;
                }

                friend stream_type &operator<<(stream_type &out,formattible const &fmt)
                {
                    fmt.writer_(out,fmt.pointer_);
                    return out;
                }

            private:
                static void void_write(stream_type &output,void const *ptr)
                {
                    CharType empty_string[1]=0;
                    output<<empty_string;
                }

                template<typename Type>
                static void write(stream_type &output,void const *ptr)
                {
                    output << *reinterpret_cast<Type const *>(ptr);
                }
                
                void const *pointer_;
                writer_type writer_;
            }; // formattible
    
            class BOOST_LOCALE_DECL format_parser  {
            public:
                format_parser(std::ios_base &ios);
                ~format_parser();
                unsigned get_posision();
                void set_flags(std::string const &format);
                void restore();
            private:
                void set_one_flag(std::string const &flag);
                void set_one_flag(std::string const &key,std::string const &value);

                format_parser(format_parser const &);
                void operator=(format_parser const &);

                std::ios_base &ios_;
                struct data;
                std::auto_ptr<data> d;
            };

        }

        template<typename CharType>
        class basic_format {
            typedef CharType char_type;
            typedef details::formattible<CharType> formattible_type;
            typedef std::basic_string<CharType> string_type;
            typedef std::basic_ostream<CharType> stream_type;


            basic_format(string_type format_string) : 
                format_(format_string),
                translate_(false)
            {
            }
            basic_format(message const &trans) : 
                message_(trans),
                translate_(true)
            {
            }

            template<typename Formattible>
            basic_format &operator % (Formattible const &object)
            {
                add(formattible_type(object));
            }

            string_type str(std::locale const &loc = std::locale()) const
            {
                std::basic_ostringstream<CharType> buffer;
                buffer.imbue(loc);
                write(buffer,false);
                return buffer.str();
            }

            void write(stream_type &out,bool save_format = true) const
            {
                string_type format;
                if(translate_)
                    format = message_.str<CharType>(out.getloc(),ext_value(out,flags::domain_id));
                else
                    format = format_;
               
                format_output(out,format);

            }
                        
            
        private:
            
            string_type widen(char const *ptr,string_type &out) const
            {
                string_type tmp;
                tmp.reserve(10);
                while(*ptr)
                    tmp+=out.widen(*ptr++);
                return tmp;
            }

            void format_output(stream_type &out,stream_type const &format) const
            {
                char_type obrk=out.widen('{');
                char_type cbrk=out.widen('}');
                size_t pos = 0;
                while(pos<=format.size()) {
                    if(format[pos]!=obrk) {
                        out<<format[pos];
                        pos++;
                        continue;
                    }
                    else if(pos+1 < format.size() && format[pos+1]==obrk) {
                        out << obrk;
                        pos+=2;
                    }
                    else {
                        pos++;
                        size_t begin=pos;
                        size_t end = format.find(begin,cbrk);
                        if(end!=std::string::npos) {
                            pos=end+1;
                            format_signle(out,format.substr(begin,end-begin-1));
                        }
                        else {
                            pos=end;
                        }
                        
                    }
                }
            }

            std::string narrow(string_type const &str,stream_type &out) const
            {
                std::string tmp;
                tmp.reserve(str.size());
                for(unsigned i=0;i<str.size();i++)
                    tmp+=out.narrow(str[i],' ');
                return tmp;
            }

            void format_signle(stream_type &out,string_type const &format) const
            {
                details::format_parser fmt(out);
                
                fmt.set_flags(narrow(format));

                unsigned position = fmt.get_posision();

                out << get(position);

                fmt.restore();

            }

       
            //
            // Non-copyable 
            //
            basic_format(basic_format const &other);
            void operator=(basic_format const &other);

            void add(formattible_type const &param)
            {
                if(parameters_count_ >= base_params_)
                    ext_params_.push_back(param);
                else 
                    parameters_[parameters_count_] = param;
                parameters_count_++;
            }

            formattible_type get(unsigned id)
            {
                if(id >= parameters_count_)
                    return formattible_type();
                else if(id >= base_params_)
                    return ext_params_[id - base_params_];
                else
                    return parameters_[id];
            }



            static const int base_params_ = 8;
            
            message message_;
            string_type format_;
            bool translate_;


            formattible_type parameters_[base_params_];
            unsigned parameters_count_;
            std::vector<formattible_type> ext_params_;
        };

    }
}


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

