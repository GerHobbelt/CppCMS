//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_COLLATOR_HPP_INCLUDED
#define BOOST_LOCALE_COLLATOR_HPP_INCLUDED

#include <locale>
#include <boost/locale/config.hpp>

namespace boost {
namespace locale {

    class info;

    ///
    /// \brief a base class that included collation level flags
    ///

    class collator_base {
    public:
        ///
        /// Unicode collation level types
        ///
        typedef enum {
            primary     = 0, ///!< 1st collation level: base letters
            secondary   = 1, ///!< 2nd collation level: letters and accents
            tertiary    = 2, ///!< 3rd collation level: letters, accents and case
            quaternary  = 3, ///!< 4th collation level: letters, accents, case and punctuation
            identical   = 4  ///!< identical collation level: include code-point comparison
        } level_type;
    };
    
    ///
    /// Collation facet. It reimplements standard C++ stc::collate
    /// allowing usage of std::locale class for direct string comparison
    ///
    template<typename CharType>
    class collator : 
        public std::collate<CharType>,
        public collator_base
    {
    public:
        typedef CharType char_type;
        typedef std::basic_string<CharType> string_type;
        

        ///
        /// Compare two strings in rage [b1,e1), [b2,e2) according using a collation level \a level
        ///
        int compare(level_type level,
                    char_type const *b1,char_type const *e1,
                    char_type const *b2,char_type const *e2) const
        {
            return do_compare(level,b1,e1,b2,e2);
        }
        ///
        /// Create a binary string that can be compared to other, usefull for collation of multiple
        /// strings for text in range [b,e)
        ///
        string_type transform(level_type level,char_type const *b,char_type const *e) const
        {
            return do_transform(level,b,e);
        }

        ///
        /// Calculate a hash that can be used for collation sensitive string comparison of a text in range [b,e)
        ///
        long hash(level_type level,char_type const *b,char_type const *e) const
        {
            return do_hash(level,b,e);
        }

        ///
        /// Compare two strings \a l and \a r using collation level \a level
        ///
        int compare(level_type level,string_type const &l,string_type const &r) const
        {
            return do_compare(level,l.data(),l.data()+l.size(),r.data(),r.data()+r.size());
        }

        ///
        /// Calculate a hash that can be used for collation sensitive string comparison of a string \a s
        ///

        long hash(level_type level,string_type const &s) const
        {
            return do_hash(level,s.data(),s.data()+s.size());
        }
        ///
        /// Create a binary string that can be compared to other, usefull for collation of multiple
        /// strings for string  \a s
        ///
        string_type transform(level_type level,string_type const &s) const
        {
            return do_transform(level,s.data(),s.data()+s.size());
        }

        ///
        /// A static member used for creation of collator instances, generally called by a generator class.
        /// 
        static collator<CharType> *create(info const &inf);
        
    protected:

        collator(size_t refs = 0) : std::collate<CharType>(refs) 
        {
        }
        virtual ~collator()
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
    BOOST_LOCALE_DECL collator<char> *collator<char>::create(info const &inf);
    #ifndef BOOST_NO_STD_WSTRING
    template<>
    BOOST_LOCALE_DECL collator<wchar_t> *collator<wchar_t>::create(info const &inf);
    #endif
    
    #ifdef BOOST_HAS_CHAR16_T
    template<>
    BOOST_LOCALE_DECL collator<char16_t> *collator<char16_t>::create(info const &inf);
    #endif
    
    #ifdef BOOST_HAS_CHAR32_T
    template<>
    BOOST_LOCALE_DECL collator<char32_t> *collator<char32_t>::create(info const &inf);
    #endif

    ///
    /// This class can be used in STL algorithms and containers for comparison of strings
    /// with different level then primary
    ///
    template<typename CharType,collator_base::level_type default_level = collator_base::primary>
    struct comparator
    {
    public:
        ///
        /// Create a comparator class for locale \a l and with collation leval \a level
        ///
        comparator(std::locale const &l=std::locale(),collator_base::level_type level=default_level) : 
            locale_(l),
            level_(level)
        {
        }

        ///
        /// Compare two strings -- equivalent to return left < right according to collation rules
        ///
        bool operator()(std::basic_string<CharType> const &left,std::basic_string<CharType> const &right) const
        {
            return std::use_facet<collator<CharType> >(locale_).compare(level_,left,right) < 0;
        }
    private:
        std::locale locale_;
        collator_base::level_type level_;
    };


}
} // boost::locale
#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
