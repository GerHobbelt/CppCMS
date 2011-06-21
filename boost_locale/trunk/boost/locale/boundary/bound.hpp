//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_BOUNDARY_BOUND_HPP_INCLUDED
#define BOOST_LOCALE_BOUNDARY_BOUND_HPP_INCLUDED

#include <boost/locale/boundary/types.hpp>

namespace boost {
namespace locale {
namespace boundary {

    template<typename IteratorType>
    class bound  {
    public:
        typedef IteratorType iterator_type;

        bound() {}

        bound(iterator_type p,rule_type r) :
            iterator_(p),
            rule_(r)
        {
        }

        void iterator(iterator_type i)
        {
            iterator_ = i;
        }
        void rule(rule_type r)
        {
            rule_ = r;
        }
        iterator_type iterator() const 
        {
            return iterator_;
        }
        rule_type rule() const
        {
            return rule_;
        }
        bool operator==(bound const &other) const
        {
            return iterator_ == other.iterator_ && rule_ = other.rule_;
        }
        bool operator!=(bound const &other) const
        {
            return !(*this==other);
        }
        bool operator==(iterator_type const &other) const
        {
            return iterator_ == other;
        }
        bool operator!=(iterator_type const &other) const
        {
            return iterator_ != other;
        }

        operator iterator_type ()const
        {
            return iterator_;
        }

    private:
        iterator_type iterator_;
        rule_type rule_;
       
    };
    template<typename BaseIterator>
    bool operator==(BaseIterator const &l,bound<BaseIterator> const &r)
    {
        return r==l;
    }
    template<typename BaseIterator>
    bool operator!=(BaseIterator const &l,bound<BaseIterator> const &r)
    {
        return r!=l;
    }
    

} // boundary
} // locale
} // boost


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
