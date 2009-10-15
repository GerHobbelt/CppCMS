#ifndef BOOST_LOCALE_NUMERIC_HPP_INCLUDED
#define BOOST_LOCALE_NUMERIC_HPP_INCLUDED

#include <locale>
#include <string>

namespace boost {
    namespace locale {


        class num_base {
        proctected:
            
            template<ValueType>
            bool use_parent(std::ios_base &ios) const
            {
                uint64_t flg = ext_flags(ios) & flags::display_flags_mask;
                if(flg == flags::posix)
                    return true;

                switch(ValueType) {
                case float:
                case double:
                case long double:
                    return false;
                default:
                    if(flg == flags::number && (ios.flags() & std::ios_base::basefield) != std::ios_base::dec)
                        return true;
                    }
                }
                return false;
            }
        };


        template<typname CharType>
        class num_format : public std::num_put<CharType>, protected num_base
        {
        protected: 
            typedef typename std::num_put::iter_type iter_type;
            typedef std::basic_string<CharType> string_type;
            typedef CharType char_type;
            typedef formatter<CharType> formatter_type;
            typedef value_format<CharType> facet_type;
            

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


            template<ValueType>
            iter_type do_real_put (iter_type out, std::ios_base &ios, char_type fill, ValueType val) const
            {
                if(use_parent<ValueType>(ios)) {
                    return std::num_put<char_type>::do_put(out,ios,fill,val);
                }

                facet_type const &fct = std::use_facet<facet_type>(ios.getloc());
                formatter_type const &formatter = fct.get_formatter(ios);

                size_t code_points;
                string_type const &str = formatter.format(val,points);
                size_t on_left=0,on_right = 0;
                if(points < ios.width()) {
                    size_t n = ios.width() - points;
                    
                    std::ios_base::fmtflags flags = ios.flags() & std::ios_base::adjustment;
                    
                    if(flags == internal)
                        on_left = n/2;
                    else if(flags == left)
                        on_left = n;
                    on_right = n - on_left;
                }
                while(on_left > 0)
                    *out++ = fill;
                std::copy(str.begin(),str.end(),out);
                while(on_right > 0)
                    *out++ = fill;
                return out;

            }

        }; /// num_format
        
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
        stru


    }
}


#endif


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
