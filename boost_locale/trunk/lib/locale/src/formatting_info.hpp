#ifndef BOOST_SRC_LOCALE_FORMATTING_INFO_HPP_INCLUDED
#define BOOST_SRC_LOCALE_FORMATTING_INFO_HPP_INCLUDED

#include <boost/variant.hpp>

namespace boost {
    namespace locale {
        namespace impl {

            struct string_set {
                template<typename Char> 
                void set(std::basic_string<Char> const &s);

                template<typename Char> 
                std::basic_string<Char> get() const;

                template<typename Char>
                string_set const &operator=(std::basic_string<Char> const &s)
                {
                    set(s);
                }

            public:

                std::string str_;
                #ifndef BOOST_NO_STD_WSTRING
                std::wstring wstr_;
                #endif
                #ifdef BOOST_HAS_CHAR16_T
                std::u16string u16str_;
                #endif
                #ifdef BOOST_HAS_CHAR32_T
                std::u32string u32str_;
                #endif
            };

            template<> 
            inline void set(std::string const &s)
            {
                str_=s;
            }

            template<> 
            inline std::string get() const
            {
                return str_;
            }

            #ifndef BOOST_NO_STD_WSTRING
            template<> 
            inline void set(std::wstring const &s)
            {
                wstr_=s;
            }

            template<> 
            inline std::wstring get() const
            {
                return wstr_;
            }
            #endif

            #ifdef BOOST_HAS_CHAR16_T
            template<> 
            inline void set(std::u16string const &s)
            {
                u16str_=s;
            }

            template<> 
            inline std::u16string get() const
            {
                return u16str_;
            }
            #endif

            #ifdef BOOST_HAS_CHAR32_T
            template<> 
            inline void set(std::u32string const &s)
            {
                u32str_=s;
            }

            template<> 
            inline std::u32string get() const
            {
                return u32str_;
            }
            #endif


            struct ios_info {
            public:
                ios_info()
                {
                    valid_=false;
                    flags_ = 0;
                }
                
                /// never copy formatter
                ios_info(ios_info const &other) :
                    flags_(other.flags_),
                    datetime_(other.datetime_),
                    separator_(other.separator_),
                    valid_(false)
                {
                }

                /// never copy formatter
                ios_info const &operator=(ios_info const &other) const
                {
                    if(this!=&other) {
                        flags_=other.flags_;
                        datetime_=other.datetime_;
                        separator_=other.separator_;
                        valid_=false;
                    }
                    return *this;
                }

                uint64_t flags() const
                {
                    return flags_
                }
                void flags(uint64_t f)
                {
                    flags_=f;
                    valid_ = false;
                }

                template<typename Char>
                void datetime(std::basic_string<Char> const &str)
                {
                    datetime_=str;
                    valid_ = false;
                }

                template<typename Char>
                std::basic_string<Char> datetime() const
                {
                    return datetime_.get<Char>();
                }
                
                template<typename Char>
                void separator(std::basic_string<Char> const &str)
                {
                    separator_=str;
                    valid_ = false;
                }

                template<typename Char>
                std::basic_string<Char> separator() const
                {
                    return separator_.get<Char>()
                }
                
                bool valid(std::ios_base &ios)
                {
                    if( !valid_
                        || ios_flags_ != ios.flags()
                        || width_ != ios.width() 
                        || precision_ != ios.precision())
                    {
                        formatter_.reset();
                        return false;
                    }
                }

                template<typename CharType>
                formatter<CharType> const *formatter(std::ios_base &ios)
                {
                    if(!valid(ios)) {

                        formatter_ = formatter<CharType>::create(ios)

                        width_      = ios.width();
                        precision_  = ios.precision();
                        ios_flags_  = ios.flags();
                        valid_      = true;
                    }

                    return dynamic_cast<formatter<CharType> const *>(formatter_.get());
                }


            private:
                std::streamsize width_,
                std::streamsize precision_;
                std::ios_base::fmtflags ios_flags_;
                uint64_t flags_;
                          
                variant_string_type datetime_;
                variant_string_type separator_; 
                bool valid_;

                std::auto_ptr<base_formatter> formatter_;

            };

        }
    }
}



#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

