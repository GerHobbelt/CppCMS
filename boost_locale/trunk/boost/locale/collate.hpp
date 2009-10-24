#ifndef BOOST_LOCALE_COLLATE_HPP_INCLUDED
#define BOOST_LOCALE_COLLATE_HPP_INCLUDED

#include <locale>

namespace boost {
namespace locale {

    class info;

    template<typename CharType>
    class collate : public std::collate<CharType> {
    public:
        typedef CharType char_type;
        typedef std::basic_string<CharType> string_type;
        
        /// Unicode collation level types
        typedef enum {
            primary = 0,
            secondary = 1,
            tertiary = 2,
            quaternary = 3
        } level_type;

        int compare(level_type level,
                    char_type const *b1,char_type const *e1,
                    char_type const *b2,char_type const *e2) const
        {
            return do_compare(level,b1,e1,b2,e2);
        }
        std::string transform(level_type level,char_type const *b,char_type const *e) const
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
        std::string transform(level_type level,std::string const &s) const
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
        virtual std::string do_transform(char_type const *b,char_type const *e) const
        {
            return do_transform(primary,b,e);
        }
        virtual long do_hash(char_type const *b,char_type const *e) const
        {
            return do_hash(primary,b,e);
        }

        virtual int do_compare(level_type level,string_type const &l,string_type const &r) const = 0;
        virtual std::string do_transform(level_type level,char_type const *b,char_type const *e) const = 0;
        virtual long do_hash(level_type level,char_type const *b,char_type const *e) const = 0;


    };

    template<>
    collator<char> *collator<char>::create(info const &inf);
    #ifndef BOOST_NO_STD_WSTRING
    template<>
    collator<wchar_t> *collator<wchar_t>::create(info const &inf);
    #endif
    
    #ifdef BOOST_HAS_CHAR16_T
    template<>
    collator<char16_t> *collator<char16_t>::create(info const &inf);
    #endif
    
    #ifdef BOOST_HAS_CHAR32_T
    template<>
    collator<char32_t> *collator<char32_t>::create(info const &inf);
    #endif

}
} // boost::locale
#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
