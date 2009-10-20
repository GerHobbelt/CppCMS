#ifndef BOOST_LOCALE_NUMERIC_HPP_INCLUDED
#define BOOST_LOCALE_NUMERIC_HPP_INCLUDED

#include <locale>
#include <string>
#include <ios>
#include <limits>
#include <boost/locale/formatter.hpp>
#include <algorithm>

namespace boost {
    namespace locale {


        class num_base {
        protected:

            template<typename ValueType>
            static bool use_parent(std::ios_base &ios)
            {
                uint64_t flg = ext_flags(ios) & flags::display_flags_mask;
                if(flg == flags::posix)
                    return true;

                if(!std::numeric_limits<ValueType>::is_integer)
                    return false;
                if(flg == flags::number && (ios.flags() & std::ios_base::basefield) != std::ios_base::dec) {
                    return true;
                }
                return false;
            }
        };

        namespace details {
            template<typename V,int n=std::numeric_limits<V>::digits,bool integer=std::numeric_limits<V>::is_integer>
            struct cast_traits;

            template<typename v>
            struct cast_traits<v,31,true> {
                typedef int32_t cast_type;
            };
            template<typename v>
            struct cast_traits<v,32,true> {
                typedef uint32_t cast_type;
            };
            template<typename v>
            struct cast_traits<v,63,true> {
                typedef int64_t cast_type;
            };
            template<typename v>
            struct cast_traits<v,64,true> {
                typedef uint64_t cast_type;
            };
            template<typename V,int u>
            struct cast_traits<V,u,false> {
                typedef double cast_type;
            };
        }

        template<typename CharType>
        class num_format : public std::num_put<CharType>, protected num_base
        {
        public:
            typedef typename std::num_put<CharType>::iter_type iter_type;
            typedef std::basic_string<CharType> string_type;
            typedef CharType char_type;
            typedef formatter<CharType> formatter_type;

            num_format(size_t refs = 0) : std::num_put<CharType>(refs)
            {
            }
        protected: 
            

            iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, long val) const
            {
                return do_real_put(out,ios,fill,val);
            }
            iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, unsigned long val) const
            {
                return do_real_put(out,ios,fill,val);
            }
            iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, double val) const
            {
                return do_real_put(out,ios,fill,val);
            }
            iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, long double val) const
            {
                return do_real_put(out,ios,fill,val);
            }
            
            #ifndef BOOST_NO_LONG_LONG 
            iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, long long val) const
            {
                return do_real_put(out,ios,fill,val);
            }
            iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, unsigned long long val) const
            {
                return do_real_put(out,ios,fill,val);
            }
            #endif


       private:



            template<typename ValueType>
            iter_type do_real_put (iter_type out, std::ios_base &ios, char_type fill, ValueType val) const
            {
                formatter_type const *formatter = 0;
                
                if(use_parent<ValueType>(ios) || (formatter = formatter_type::get(ios)) == 0) {
                    return std::num_put<char_type>::do_put(out,ios,fill,val);
                }
                
                size_t code_points;
                typedef typename details::cast_traits<ValueType>::cast_type cast_type;
                string_type const &str = formatter->format(static_cast<cast_type>(val),code_points);
                std::streamsize on_left=0,on_right = 0,points = code_points;
                if(points < ios.width()) {
                    std::streamsize n = ios.width() - points;
                    
                    std::ios_base::fmtflags flags = ios.flags() & std::ios_base::adjustfield;
                    
                    if(flags == std::ios_base::internal)
                        on_left = n/2;
                    else if(flags == std::ios_base::left)
                        on_left = n;
                    on_right = n - on_left;
                }
                while(on_left > 0) {
                    *out++ = fill;
                    on_left--;
                }
                std::copy(str.begin(),str.end(),out);
                while(on_right > 0) {
                    *out++ = fill;
                    on_right--;
                }
                return out;

            }

        };  /// num_format
       
       
       /* 
        template<typname CharType>
        class num_parse : public std::num_get<CharType>,
        {
        protected: 
            typedef typename std::num_get::iter_type iter_type;
            typedef std::basic_string<CharType> string_type;
            typedef CharType char_type;
            typedef formatter<CharType> formatter_type;
            typedef value_format<CharType> facet_type;

        private:
            

            template<typename ValueType>
            iter_type do_real_get(iter_type in,iter_type end,std::ios_base &ios,std::ios_base::iostate &err,ValueType &val) const
            {
                if(use_parent<ValueType>(ios))
                    return std::num_get<CharType>::do_get(in,end,ios,ios,err,val);

                typedef typename traits<ValueType>::subst_type subst_type;

                subst_type value;

                string_type str=read_string(in,end,ios);
                if(in==end)
                    err |= std::ios_base::eofbit;

                facet_type const &fct = std::use_facet<facet_type>(ios.getloc());
                formatter_type const &formatter = fct.get_formatter(ios);
                if(!formatter->parse(str,value) || !traits<ValueType>::validate(value))
                    err |= std::ios_base::failbit;
                else 
                    val=static_cast<ValueType>(value);
                return in;
            }
            
        public:
            template<ValueType>
            struct traits;
        };

        template<sizeoe>
        stru*/


    }
}


#endif


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
