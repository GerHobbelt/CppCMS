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
#include <boost/assert.hpp>
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
    
    template<typename Iterator>
    class basic_text_range;
    
    template<typename CharType>
    class basic_text_iterator;
    
    template<typename IteratorType>
    IteratorType dynamic_text_iterator_cast(basic_text_iterator<typename IteratorType::value_type> const &in);
    template<typename IteratorType>
    IteratorType static_text_iterator_cast(basic_text_iterator<typename IteratorType::value_type> const &in);

    namespace details {
        template<typename CharType>
        class abstract_iterator;
    }
    
    template<typename ObjectType>
    class clone_ptr {
    public:
        clone_ptr() : p_(0) 
        {
        }
        ~clone_ptr()
        {
            delete p_;
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
    class basic_text_iterator;

    template<typename CharType>
    class basic_abstract_text_range {
    public:
        typedef CharType char_type;
        typedef basic_text_iterator<CharType> iterator_type;
        
        basic_abstract_text_range() :
            fast_buffer_(0)
        {
        }
        virtual basic_abstract_text_range *clone() const = 0;
        virtual iterator_type begin() = 0;
        virtual iterator_type end() = 0;
    protected:
        char_type const *fast_buffer_;
        friend class basic_text_iterator<char_type>;
    };
        
    template<typename CharType,typename SomeIteratorType>
    struct linear_iterator_traits {
        static const bool is_linear = false;
    };

    template<typename CharType>
    struct linear_iterator_traits<CharType,typename std::basic_string<CharType>::iterator> {
        static const bool is_linear = true;
    };

    template<typename CharType>
    struct linear_iterator_traits<CharType,typename std::basic_string<CharType>::const_iterator> {
        static const bool is_linear = true;
    };
    
    template<typename CharType>
    struct linear_iterator_traits<CharType,typename std::vector<CharType>::iterator> {
        static const bool is_linear = true;
    };

    template<typename CharType>
    struct linear_iterator_traits<CharType,typename std::vector<CharType>::const_iterator> {
        static const bool is_linear = true;
    };

    template<typename CharType>
    struct linear_iterator_traits<CharType,CharType *> {
        static const bool is_linear = true;
    };

    template<typename CharType>
    struct linear_iterator_traits<CharType,CharType const *> {
        static const bool is_linear = true;
    };


    namespace details {

        template<typename ValueType>
        class abstract_iterator {
            void operator=(abstract_iterator const &);
        public:
            abstract_iterator() :
                current_position(0)
            {
            }
            
            size_t current_position;

            virtual abstract_iterator *clone() const = 0;
            virtual ValueType const &do_dereference(size_t position) = 0;
            virtual ~abstract_iterator() 
            {
            }
        };
        template<typename T>
        class real_iterator : public abstract_iterator<typename std::iterator_traits<T>::value_type> {
        public:
            typedef typename std::iterator_traits<T>::value_type char_type;
            virtual real_iterator *clone() const { return new real_iterator(*this); }
            real_iterator(T i,size_t p) : it(i) 
            {
                this->current_position = p;
            }
            char_type const &do_dereference(size_t position) 
            {
                std::advance(it,ptrdiff_t(position - this->current_position));
                this->current_position = position;
                return *it;
            }
            T it;
        };

        template<typename IteratorType,typename Tag=typename std::iterator_traits<IteratorType>::iterator_category>
        struct distance_traits {
            static ptrdiff_t distance(IteratorType begin,IteratorType end,IteratorType start,IteratorType target)
            {
                IteratorType start_forward = start;
                IteratorType start_backward = start;
                ptrdiff_t count = 0;
                while(start_forward!=target && start_backward!=target) {
                    if(start_forward!=end)
                        ++start_forward;
                    if(start_backward!=begin)
                        --start_backward;
                    count++;
                }
                if(start_forward==target)
                    return count;
                else
                    return -count;
            }
        };

        template<typename IteratorType>
        struct distance_traits<IteratorType,std::random_access_iterator_tag> {
            static ptrdiff_t distance(IteratorType /*begin*/,IteratorType /*end*/,IteratorType start,IteratorType target)
            {
                return target - start;
            }
        };

    } // details

    template<typename CharType>
    class basic_text_iterator : public boost::iterator_facade<
                                            basic_text_iterator<CharType>,
                                            CharType,
                                            boost::random_access_traversal_tag,
                                            CharType const &
                                        >
    {
    public:
        typedef CharType char_type;
        typedef basic_abstract_text_range<char_type> range_type;
        
        basic_text_iterator() : 
            range_(0)
        {
        }
        
        basic_text_iterator(range_type *r,size_t pos) :
            range_(r),
            position_(pos)
        {
        }

        template<typename It>
        basic_text_iterator(range_type *r,size_t pos,It i) :
            range_(r),
            position_(pos),
            it_(new details::real_iterator<It>(i,pos))
        {
        }


        char_type const &dereference() const
        {
            if(it_.get())
                return it_->do_dereference(position_);
            return range_->fast_buffer_[position_];
        }

        bool equal(basic_text_iterator const &other) const
        {
            return position_ == other.position_;
        }
        void increment()
        {
            position_++;
        }
        void decrement()
        {
            position_--;
        }
        void advance(ptrdiff_t n)
        {
            position_+=n;
        }
        ptrdiff_t distance_to(basic_text_iterator const &other) const
        {
            return other.position_ - position_;
        }
    private:
        friend class basic_abstract_text_range<char_type>;

        template<typename IteratorType>
        friend class basic_text_range;

        template<typename IteratorType>
        friend IteratorType dynamic_text_iterator_cast(basic_text_iterator<typename IteratorType::value_type> const &in);

        template<typename IteratorType>
        friend IteratorType static_text_iterator_cast(basic_text_iterator<typename IteratorType::value_type> const &in);

        // Actual range
        range_type const *range_;
        size_t position_;
        clone_ptr<details::abstract_iterator<char_type> > it_;
    };

    template<typename Iterator>
    class basic_text_range : public basic_abstract_text_range<typename std::iterator_traits<Iterator>::value_type>
    {
    public:
        typedef basic_abstract_text_range<typename std::iterator_traits<Iterator>::value_type> base_type;
        typedef typename std::iterator_traits<Iterator>::value_type char_type;
        typedef basic_text_iterator<char_type> iterator_type;
        basic_text_range(Iterator b,Iterator e) :
            begin_(b),
            end_(e)
        {
            if(linear_iterator_traits<char_type,Iterator>::is_linear) {
                if(begin_ != end_)
                    this->fast_buffer_ = &*begin_;
                else {
                    static const char_type tmp = 0;
                    this->fast_buffer_ = &tmp;
                }
            }
            size_ = std::distance(begin_,end_);
        }
        virtual basic_text_range *clone() const
        {
            return new basic_text_range(*this);
        }
        iterator_type begin()
        {
            if(this->fast_buffer_) {
                iterator_type it(this,0);
                return it;
            }
            else {
                iterator_type it(this,0,begin_);
                return it;
            }
        }
        iterator_type end()
        {
            if(this->fast_buffer_) {
                iterator_type it(this,size_);
                return it;
            }
            else {
                iterator_type it(this,size_,end_);
                return it;
            }
        }
    private:
       
        Iterator begin_;
        Iterator end_;
        size_t size_;
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


    } // details

    template<typename IteratorType>
    IteratorType dynamic_text_iterator_cast(basic_text_iterator<typename IteratorType::value_type> const &in)
    {
        if(in.fast_buffer_) {
            typedef basic_text_range<IteratorType> range_type;
            range_type const &r=dynamic_cast<range_type const &>(*in.range_);
            IteratorType result = r.begin_;
            std::advance(result,in.position_);
            return result;
        }
        else {
            typedef details::real_iterator<IteratorType> iterator_type;
            iterator_type const &it = dynamic_cast<iterator_type const &>(*in.it_);
            IteratorType result = it.it;
            ptrdiff_t diff = in.position_ - it.current_position;
            std::advance(result,diff);
            return result;
        }
    }
    
    template<typename IteratorType>
    IteratorType static_text_iterator_cast(basic_text_iterator<typename IteratorType::value_type> const &in)
    {
        if(in.fast_buffer_) {
            typedef basic_text_range<IteratorType> range_type;
            range_type const &r=static_cast<range_type const &>(*in.range_);
            IteratorType result = r.begin_;
            std::advance(result,in.position_);
            return result;
        }
        else {
            typedef details::real_iterator<IteratorType> iterator_type;
            iterator_type const &it = static_cast<iterator_type const &>(*in.it_);
            IteratorType result = it.it;
            ptrdiff_t diff = in.position_ - it.current_position;
            std::advance(result,diff);
            return result;
        }
    }
    
    template<typename CharType>
    class basic_text {
    public:
        typedef basic_abstract_text_range<CharType> range_type;
        typedef basic_text_iterator<CharType> iterator;
        iterator begin()
        {
            return range_->begin();
        }
        iterator end()
        {
            return range_->end();
        }
        template<typename IteratorType>
        basic_text(IteratorType b,IteratorType e)
        {
            range_.reset(new basic_text_range<IteratorType>(b,e));
        }
    private:
        clone_ptr<range_type> range_;
    };

    typedef basic_text<char> text;
    typedef basic_text<wchar_t> wtext;

    template<typename Iterator>
    ptrdiff_t advance_in_range(Iterator &it,ptrdiff_t n,Iterator const &l,Iterator const &h)
    {
        return details::advance_traits<Iterator>::advance(it,n,l,h);
    }


} // locale 
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
