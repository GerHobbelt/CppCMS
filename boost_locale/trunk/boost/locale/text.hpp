//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_TEXT_HPP
#define BOOST_LOCALE_TEXT_HPP

#include <boost/iterator/iterator_facade.hpp>
#include <numeric>
#include <iterator>
#include <vector>
#include <string>

namespace boost {
namespace locale {
    
    template<typename CharType>
    class basic_abstract_text_range_iterator;
    
    template<typename CharType>
    class basic_abstract_text_range;
    
    template<typename CharType>
    class basic_text_iterator;
    
    template<typename ObjectType>
    class clone_ptr {
    public:
        clone_ptr() : p_(0) 
        {
        }
        explicit clone_ptr(ObjectType *p) : p_(p)
        {
        }
        clone_ptr(clone_ptr const &other) : p_(0)
        {
            if(other.p_!=0)
                p_  = other.p_->clone();
        }
        clone_ptr const &operator=(clone_ptr const &other) 
        {
            if(this!=&other) {
                clone_ptr tmp(other);
                swap(tmp);
            }
            return *this;
        }
        void swap(clone_ptr &other)
        {
            std::swap(p_,other.p_);
        }
        void reset(ObjectType *p)
        {
            delete p_;
            p_ = p;
        }
        ObjectType *operator->() const
        {
            return p_;
        }
        ObjectType &operator*() const
        {
            return *p_;
        }
        ObjectType *get() const
        {
            return p_;
        }
    private:
        ObjectType *p_;
    };


    template<typename CharType>
    class basic_abstract_text_range {
    public:
        basic_abstract_text_range() : 
            end_pos_(-1) // npos
        {
        }
        virtual basic_abstract_text_range *clone() const = 0;
        virtual basic_abstract_text_range_iterator<CharType> *begin() = 0;
        virtual basic_abstract_text_range_iterator<CharType> *end() = 0;
    protected:
        friend class basic_abstract_text_range_iterator<CharType>;
        size_t end_pos_;
    };

    template<typename CharType>
    class basic_abstract_text_range_iterator {
    public:
        typedef CharType char_type;
        typedef basic_abstract_text_range<CharType> text_range_type;

        static const size_t npos = -1;

        basic_abstract_text_range_iterator(text_range_type *r) :
            range_(r)
        {
        }
        
        char_type const &dereference() const
        {
            return current_;
        }
        bool equal(basic_abstract_text_range_iterator const &other) const
        {
            return normalize_position(position_) == normalize_position(other.position_);
        }
        void increment()
        {
            do_increment();
        }
        void decrement() 
        {
            do_decrement();
        }
        void advance(ptrdiff_t n) 
        {
            do_advance(n);
        }

        ptrdiff_t advance(  ptrdiff_t n,
                            basic_abstract_text_range_iterator const &low,
                            basic_abstract_text_range_iterator const &high)
        {
            return do_advance(n,low,high);
        }

        virtual basic_abstract_text_range_iterator *clone() const = 0;

        char_type *copy(basic_abstract_text_range_iterator const &to,char_type *begin,char_type const *end) const
        {
            return do_copy(to,begin,end);
        }

        ptrdiff_t distance_to(basic_abstract_text_range_iterator const &other) const
        {
            size_t pos = normalize_position(position_);
            size_t other_pos = normalize_position(other.position_);
            
            if((pos != npos && other_pos != npos) || (pos==other_pos)) {
                if(pos < other_pos)
                    return other_pos - pos;
                else
                    return -static_cast<ptrdiff_t>(pos - other_pos);
            }

            return do_distance_to(other);
        }
        
    protected:

        size_t normalize_position(size_t pos) const
        {
            if(pos == npos && range_->end_pos_!=npos)
                pos = range_->end_pos_;
            return pos;
        }

        virtual void do_increment() = 0;
        virtual void do_decrement() = 0;
        virtual void do_advance(ptrdiff_t n) = 0;
        virtual ptrdiff_t do_distance_to(basic_abstract_text_range_iterator const &other) const = 0;
        virtual ptrdiff_t do_advance(   ptrdiff_t n,
                                        basic_abstract_text_range_iterator const &low,
                                        basic_abstract_text_range_iterator const &high) = 0;
        virtual char_type *do_copy( basic_abstract_text_range_iterator const &to,
                                    char_type *begin,
                                    char_type const *end) const = 0;
        
        size_t position_; 
        char_type current_;
        text_range_type *range_;
    };

    template<typename Iterator>
    class basic_text_range_iterator;

    template<typename Iterator>
    class basic_text_range : public basic_abstract_text_range<typename std::iterator_traits<Iterator>::value_type> 
    {
    public:

        typedef Iterator iterator_type;

        basic_text_range(Iterator b,Iterator e) :
            begin_(b),
            end_(e)
        {
        }
        basic_text_range *clone() const 
        {
            return new basic_text_range(*this);
        }

        basic_text_range_iterator<Iterator> *begin();
        basic_text_range_iterator<Iterator> *end();

        iterator_type real_begin() const 
        {
            begin_;
        }
        iterator_type real_end() const 
        {
            end_;
        }

    protected:
        friend class basic_text_range_iterator<Iterator>;
        iterator_type begin_;
        iterator_type end_;
    };

    namespace details {
        template<typename Iterator,typename Tag=typename std::iterator_traits<Iterator>::iterator_category>
        struct advance_traits {
            static ptrdiff_t advance(Iterator &it,ptrdiff_t n,Iterator const &begin,Iterator const &end)
            {
                ptrdiff_t count = 0;
                while(n > 0 && it != end) {
                    ++it;
                    n--;
                    count ++;
                }
                while(n < 0 && it != begin) {
                    --it;
                    n++;
                    count --;
                }
                return count;
            }
        };

        template<typename Iterator>
        struct advance_traits<Iterator,std::random_access_iterator_tag> {
            static ptrdiff_t advance(Iterator &it,ptrdiff_t n,Iterator const &begin,Iterator const &end)
            {
                if(n > 0) {
                    ptrdiff_t dist_to_end = end - it;
                    if(dist_to_end < n)
                        n=dist_to_end;
                }
                else {
                    ptrdiff_t dist_to_begin = begin - it;
                    if(n < dist_to_begin)
                        n=dist_to_begin;
                }
                it+=n;
                return n;
            }
        };

        template<typename CharType>
        struct advance_traits<basic_text_iterator<CharType>,std::random_access_iterator_tag> {
            typedef basic_text_iterator<CharType> iterator;
            static ptrdiff_t advance(iterator &it,ptrdiff_t n,iterator const &begin,iterator const &end)
            {
                return it.advance(n,begin,end);
            }
        };
    } // details


    template<typename Iterator> 
    class basic_text_range_iterator : public basic_abstract_text_range_iterator<typename std::iterator_traits<Iterator>::value_type> 
    {
    public:
        typedef basic_text_range<Iterator> range_type;
        typedef typename std::iterator_traits<Iterator>::value_type char_type;
        static const size_t npos = basic_abstract_text_range_iterator<typename std::iterator_traits<Iterator>::value_type>::npos;

        basic_text_range_iterator(range_type *r,bool begin) : 
            basic_abstract_text_range_iterator<typename std::iterator_traits<Iterator>::value_type>(r)
        {
            if(begin) {
                it_ = r->begin_;
                this->position_ = 0;
                update_current();
            }
            else {
                it_ = r->end_;
                this->position_ = r->end_pos_;
            }
        }
        virtual basic_text_range_iterator *clone() const
        {
            return new basic_text_range_iterator(*this);
        }
        virtual void do_increment() 
        {
            ++it_;
            ++this->position_;
            update_current();
        }
        virtual void do_decrement() 
        {
            if(this->position_ == npos) {
                if(range()->end_pos_ == npos) 
                    range()->end_pos_ = std::distance(range()->begin_,range()->end_);
                this->position_ = range()->end_pos_;
            }
            --it_;
            --this->position_;
            this->current_ = *it_;
        }
        virtual ptrdiff_t do_advance(   ptrdiff_t n,
                                        basic_abstract_text_range_iterator<char_type> const &base_low,
                                        basic_abstract_text_range_iterator<char_type> const &base_high)
        {
            basic_text_range_iterator const &low = static_cast<basic_text_range_iterator const &>(base_low);
            basic_text_range_iterator const &high = static_cast<basic_text_range_iterator const &>(base_high);
            ptrdiff_t diff = details::advance_traits<Iterator>::advance(it_,n,low.it_,high.it_);
            this->position_+=diff;
            update_current();
            return diff;
        }
        virtual void do_advance(ptrdiff_t n) 
        {
            std::advance(it_,n);
            this->position_+=n;
            update_current();
        }
        virtual ptrdiff_t do_distance_to(basic_abstract_text_range_iterator<char_type> const &other_base) const 
        {
            basic_text_range_iterator const &other = static_cast<basic_text_range_iterator const &>(other_base);
            ptrdiff_t dist = 0;
            if(this->position_ < other.position_) {
                dist = std::distance(it_,other.it_);
                if(range()->end_pos_ != npos && other.it_ == range()->end_ ) {
                    range()->end_pos_ = this->position_ + dist;
                }
            }
            else {
                dist = - std::distance(other.it_,it_);
                if(range()->end_pos_ != npos && it_ == range()->end_ ) {
                    range()->end_pos_ = other.position_ - dist;
                }
            }
            return dist;

        }
        virtual char_type *do_copy( basic_abstract_text_range_iterator<char_type> const &base_to,
                                    char_type *begin,
                                    char_type const *end) const 
        {
            Iterator from = it_;
            Iterator to = static_cast<basic_text_range_iterator const &>(base_to).it_;
            while(from!=to && begin!=end) {
                *begin++ = *from++;
            }
            return begin;
        }

        Iterator real() const
        {
            return it_;
        }

    private:

        void update_current()
        {
            if(it_!=range()->end_)
                this->current_ = *it_;
            else
                range()->end_pos_ = this->position_;
        }
        range_type *range() const
        {
            return static_cast<range_type *>(this->range_);
        }

        Iterator it_;
    };

    template<typename Iterator>
    basic_text_range_iterator<Iterator> *basic_text_range<Iterator>::begin() 
    {
        return new basic_text_range_iterator<Iterator>(this,true);
    }

    template<typename Iterator>
    basic_text_range_iterator<Iterator> *basic_text_range<Iterator>::end() 
    {
        return new basic_text_range_iterator<Iterator>(this,false);
    }

    template<typename CharType>
    class basic_text_iterator : 
        public boost::iterator_facade<
                        basic_text_iterator<CharType> ,
                        CharType,
                        boost::random_access_traversal_tag,
                        CharType const &
                    >
    {
    public:
        typedef basic_abstract_text_range<CharType> text_range;
        typedef basic_abstract_text_range_iterator<CharType> text_range_iterator;
        typedef CharType char_type;

        basic_text_iterator() 
        {
        }
        basic_text_iterator(text_range_iterator *it) : it_(it)
        {
        }
        
        text_range_iterator const &get_range_iterator() const
        {
            return  *it_;
        }
        char_type const &dereference() const
        {
            return it_->dereference();
        }
        bool equal(basic_text_iterator const &other) const
        {
            return it_->equal(*other.it_);
        }
        void increment()
        {
            it_->increment();
        }
        void decrement()
        {
            it_->decrement();
        }
        ptrdiff_t advance(ptrdiff_t n,basic_text_iterator const &b,basic_text_iterator const &e)
        {
            return it_->advance(n,*b.it_,*e.it_);
        }
        void advance(ptrdiff_t n)
        {
            it_->advance(n);
        }
        ptrdiff_t distance_to(basic_text_iterator const &other) const
        {
            return it_->distance_to(*other.it_);
        }

        text_range_iterator const &real_iterator() const 
        {
            return *it_;
        }
        text_range_iterator &real_iterator() 
        {
            return *it_;
        }
        
    private:
        clone_ptr<text_range_iterator> it_;
    };

    template<typename CharType>
    class basic_text {
    public:
        typedef basic_text_iterator<CharType> iterator;
        typedef CharType char_type;

        basic_text()
        {
        }

        template<typename IteratorType>
        basic_text(IteratorType b,IteratorType e)
        {
            range_.reset(new basic_text_range<IteratorType>(b,e));
        }

        iterator begin()
        {
            return iterator(range_->begin());
        }
        iterator end()
        {
            return iterator(range_->end());
        }

        std::pair<CharType const *,CharType const *> get_range_as_pointers()
        {
            std::pair<CharType const *,CharType const *> result;
            
            result.first = 0;
            result.second = 0;

            if( 
                cast_to<char_type *>(result)
                || cast_to<char_type const *>(result)
                || cast_to<typename std::vector<char_type>::iterator>(result)
                || cast_to<typename std::vector<char_type>::const_iterator>(result)
                || cast_to<typename std::basic_string<char_type>::iterator>(result)
                || cast_to<typename std::basic_string<char_type>::const_iterator>(result) 
            )
            {
                return result;
            }
            return result;
        }
        
    private:
        template<typename IteratorType>
        bool cast_to(std::pair<CharType const *,CharType const *> &result)
        {
            typedef basic_text_range<IteratorType> real_range_type;
            real_range_type *real_range = dynamic_cast<real_range_type *>(range_.get());
            if(!real_range)
                return false;
            IteratorType begin = real_range->real_begin();
            IteratorType end = real_range->real_end();
            if(begin!=end) {
                result.first = &*begin;
                result.second = result.first + (end - begin);
            }
            else {
                static CharType tmp;
                result.first = &tmp;
                result.second = &tmp;
            }
            return true;
        }
        clone_ptr<basic_abstract_text_range<CharType> > range_;
    };

    namespace details {
        template<typename IteratorIn>
        struct copy_range_traits {
            
            typedef typename std::iterator_traits<IteratorIn>::value_type char_type;
            typedef IteratorIn iterator_type;
            
            static char_type *copy(iterator_type ib,iterator_type const &ie,char_type *ob,char_type const *oe)
            {
                while(ib!=ie && ob!=oe)
                    *ob++ = *ib++;
                return ob;
            }
        };
        template<typename CharType>
        struct copy_range_traits<basic_text_iterator<CharType> > {
            typedef CharType char_type;
            typedef basic_text_iterator<CharType> iterator_type;

            static char_type *copy(iterator_type const &ib,iterator_type const &ie,char_type *ob,char_type const *oe)
            {
                return ib.get_range_iterator().copy(ie.get_range_iterator(),ob,oe);
            }
        };
    }

    template<typename IteratorType>
    typename std::iterator_traits<IteratorType>::value_type 
    *copy_range(IteratorType const &ib,
                IteratorType const &ie,
                typename std::iterator_traits<IteratorType>::value_type *ob,
                typename std::iterator_traits<IteratorType>::value_type const *oe)
    {
        return details::copy_range_traits<IteratorType>::copy(ib,ie,ob,oe);
    }
    
    template<typename IteratorType>
    IteratorType dynamic_text_iterator_cast(basic_text_iterator<typename IteratorType::value_type> const &in)
    {
        return dynamic_cast<basic_text_range_iterator<IteratorType> const &>(in.real_iterator()).real();
    }
    
    template<typename IteratorType>
    IteratorType static_text_iterator_cast(basic_text_iterator<typename IteratorType::value_type> const &in)
    {
        return dynamic_cast<basic_text_range_iterator<IteratorType> const &>(in.real_iterator()).real();
    }

    typedef basic_text<char> text;
    typedef basic_text<wchar_t> wtext;

    namespace details {
    }

    template<typename Iterator>
    ptrdiff_t advance_in_range(Iterator &it,ptrdiff_t n,Iterator const &l,Iterator const &h)
    {
        return details::advance_traits<Iterator>::advance(it,n,l,h);
    }


} // locale 
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
