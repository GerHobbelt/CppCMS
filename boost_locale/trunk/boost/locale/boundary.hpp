#ifndef BOOST_LOCALE_BOUNDARY_HPP_INCLUDED
#define BOOST_LOCALE_BOUNDARY_HPP_INCLUDED

namespace boost {
    namespace locale {
        
        struct boundary {
            typedef enum {
                character,
                word,
                sentence,
                line
            } boundary_type;

            typedef std::pair<size_t,bool> info_type;
            typedef std::vector<info_type> index_type;

            template<typename CharType>
            index_type find_breaks_and_marks(boundary_type t,std::basic_string<CharType> const &str,std::locale const &locale = std::locale());

            template<typename CharType>
            std::basic_string<CharType> max_substring(boundary_type t,std::basic_string<CharType> const &str,size_t from,size_t n,std::locale const &locale = std::locale())
            {
                size_t to= (n==std::string::npos) ? from + str.size() : to + n;
                index_type index=find_breaks_and_marks(t,str);
                index_type::iterator l=std::lower_bound(index.begin(),index.end(),info_type(from,false),cmp);
                index_type::iterator u=std::upper_bound(index.begin(),index.end(),info_type(to,false),cmp);
                if(u==index.end())
                    return str.substr(l->first);
                else
                    return str.substr(l->first,l->second-l->first);
            }
            
            template<typename CharType>
            std::basic_string<CharType> min_substring(boundary_type t,std::basic_string<CharType> const &str,size_t from,size_t n,std::locale const &locale = std::locale())
            {
                size_t to= (n==std::string::npos) ? from + str.size() : to + n;
                index_type index=find_breaks_and_marks(t,str);
                index_type::iterator l=std::upper_bound(index.begin(),index.end(),info_type(from,false),cmp);
                index_type::iterator u=std::lower_bound(index.begin(),index.end(),info_type(to,false),cmp);
                if(u==index.end())
                    return str.substr(l->first);
                else
                    return str.substr(l->first,l->second-l->first);
            }

            static bool cmp(info_type const &l,info_type const &r)
            {
                return l.first < r.first;
            }
            
        };


        template<>
        BOOST_LOCALE_DECL index_type find_breaks_and_marks(boundary_type t,std::basic_string<char> const &str,std::locale const &locale = std::locale());
    }
}

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
