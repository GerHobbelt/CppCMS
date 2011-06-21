//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_BOUNDARY_TOKEN_HPP_INCLUDED
#define BOOST_LOCALE_BOUNDARY_TOKEN_HPP_INCLUDED
#include <boost/locale/config.hpp>
#ifdef BOOST_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4275 4251 4231 4660)
#endif
#include <locale>
#include <string>
#include <iosfwd>
#include <iterator>


namespace boost {
namespace locale {
namespace boundary {

    template<typename IteratorType>
    class token : public std::pair<IteratorType,IteratorType> {
    public:
        typedef typename std::iterator_traits<IteratorType>::value_type char_type;
        typedef std::basic_string<char_type> string_type;
        typedef char_type value_type;
        typedef IteratorType iterator;
        typedef IteratorType const_iterator;
        typedef typename std::iterator_traits<IteratorType>::difference_type difference_type;

        token() {}
        token(iterator b,iterator e,rule_type r) :
            std::pair<IteratorType,IteratorType>(b,e),
            rule_(r)
        {
        }

        IteratorType begin() const 
        {
            return this->first;
        }
        IteratorType end() const
        {
            return this->second;
        }

        template <class T, class A>
        operator std::basic_string<char_type, T, A> ()const
        {
            return std::basic_string<char_type, T, A>(this->first, this->second);
        }

        string_type str() const
        {
            return string_type(begin(),end());
        }

        size_t length() const
        {
            return std::distance(begin(),end());
        }

        bool empty() const
        {
            return begin() == end();
        }
        rule_type rule() const
        {
            return rule_;
        }
        void rule(rule_type r)
        {
            rule_ = r;
        }
    private:
        rule_type rule_;
       
    };
    
    template<typename CharType,typename TraitsType,typename Iterator>
    std::basic_ostream<CharType,TraitsType> &operator<<(
            std::basic_ostream<CharType,TraitsType> &out,
            token<Iterator> const &tok)
    {
        for(Iterator p=tok.begin(),e=tok.end();p!=e;++p)
            out << *p;
        return out;
    }

} // boundary
} // locale
} // boost

#ifdef BOOST_MSVC
#pragma warning(pop)
#endif

#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
