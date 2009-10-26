#ifndef BOOST_LOCALE_COLLATE_HPP_INCLUDED
#define BOOST_LOCALE_COLLATE_HPP_INCLUDED

#include <locale>
#include <boost/locale/config.hpp>

namespace boost {
namespace locale {

    class info;

    class collate_base {
    public:
        ///
        /// Unicode collation level types
        ///
        typedef enum {
            primary     = 0,
            secondary   = 1,
            tertiary    = 2,
            quaternary  = 3
        } level_type;
    };
    
    ///
    /// Collation facet. It reimplements standard C++ stc::collate
    /// allowing usage of std::locale class for direct string comparison
    ///
    template<typename CharType>
    class collate : 
        public std::collate<CharType>,
        public collate_base
    {
    public:
        typedef CharType char_type;
        typedef std::basic_string<CharType> string_type;
        

        ///
        /// Compare two strings according using a collation level \a level
        ///
        int compare(level_type level,
                    char_type const *b1,char_type const *e1,
                    char_type const *b2,char_type const *e2) const
        {
            return do_compare(level,b1,e1,b2,e2);
        }
        ///
        /// Create a binary string that can be compared to other, usefull for collation of multiple
        /// strings
        ///
        string_type transform(level_type level,char_type const *b,char_type const *e) const
        {
            return do_transform(level,b,e);
        }

        long hash(level_type level,char_type const *b,char_type const *e) const
        {
            return do_hash(level,b,e);
        }

        int compare(level_type level,string_type const &l,string_type const &r) const
        {
            return do_compare(level,l.data(),l.data()+l.size(),r.data(),r.data()+r.size());
        }
        long hash(level_type level,string_type const &s) const
        {
            return do_compare(level,s.data(),s.data()+s.size());
        }
        string_type transform(level_type level,string_type const &s) const
        {
            return do_transform(level,s.data(),s.data()+s.size());
        }

        static collate<CharType> *create(info const &inf);
        
    protected:

        collate(size_t refs = 0) : std::collate<CharType>(refs) 
        {
        }
        virtual ~collate()
        {
        }
        
        virtual int do_compare( char_type const *b1,char_type const *e1,
                                char_type const *b2,char_type const *e2) const
        {
            return do_compare(primary,b1,e1,b2,e2);
        }
        virtual string_type do_transform(char_type const *b,char_type const *e) const
        {
            return do_transform(primary,b,e);
        }
        virtual long do_hash(char_type const *b,char_type const *e) const
        {
            return do_hash(primary,b,e);
        }

        virtual int do_compare( level_type level,
                                char_type const *b1,char_type const *e1,
                                char_type const *b2,char_type const *e2) const = 0;
        virtual string_type do_transform(level_type level,char_type const *b,char_type const *e) const = 0;
        virtual long do_hash(level_type level,char_type const *b,char_type const *e) const = 0;


    };

    template<>
    BOOST_LOCALE_DECL collate<char> *collate<char>::create(info const &inf);
    #ifndef BOOST_NO_STD_WSTRING
    template<>
    BOOST_LOCALE_DECL collate<wchar_t> *collate<wchar_t>::create(info const &inf);
    #endif
    
    #ifdef BOOST_HAS_CHAR16_T
    template<>
    BOOST_LOCALE_DECL collate<char16_t> *collate<char16_t>::create(info const &inf);
    #endif
    
    #ifdef BOOST_HAS_CHAR32_T
    template<>
    BOOST_LOCALE_DECL collate<char32_t> *collate<char32_t>::create(info const &inf);
    #endif

}
} // boost::locale
#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
