//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_BOUNDARY_HPP_INCLUDED
#define BOOST_LOCALE_BOUNDARY_HPP_INCLUDED

#include <boost/locale/config.hpp>
#include <boost/cstdint.hpp>

#include <string>
#include <locale>
#include <vector>
#include <iterator>
#include <algorithm>

namespace boost {
    namespace locale {
        
        namespace boundary {
            ///
            /// \brief The enum that describes possible break types
            ///
            typedef enum {
                character,  ///< Find character boundaries
                word,       ///< Find word boundaries
                sentence,   ///< Find sentence boundaries
                line        ///< Find a positions suitable for line breaks
            } boundary_type;

            ///
            /// Flags used with word boundary analysis -- the type of word found
            ///
            typedef enum {
                number  = 1 << 0,   ///< Word that appear to be a number
                letter  = 1 << 1,   ///< Word that contains letters
                kana    = 1 << 2,   ///< Word that contains kana characters
                ideo    = 1 << 3,   ///< Word that contains ideographic characters
            } word_type;
            ///
            /// Flags that describe a type of line break
            ///
            typedef enum {
                soft    = 1 << 0,   ///< Optional line break
                hard    = 1 << 1    ///< Mandatory line break
            } line_break_type;
            
            ///
            /// \ brief a structure that describes break position
            /// 
            struct break_info {

                ///
                /// Default constructor -- all fields are zero
                /// 
                break_info() : 
                    offset(0),
                    next(0),
                    prev(0),
                    brk (0),
                    reserved(0)
                {
                }
                ///
                /// Create a break info with offset \a v
                ///
                break_info(unsigned v) :
                    offset(v),
                    next(0),
                    prev(0),
                    brk (0),
                    reserved(0)
                {
                }

                uint32_t offset;            ///< Offset from the beginning of the text where break occurs
                uint32_t next       : 10;   ///< The flag describing a word following the break position
                uint32_t prev       : 10;   ///< The flag describing a word preceding the break position
                uint32_t brk        : 10;   ///< The flag describing a type of line break
                uint32_t reserved   : 2;
               
                ///
                /// Comparison operator allowing usage of break_info in STL algorithms
                /// 
                bool operator<(break_info const &other) const
                {
                    return offset < other.offset;
                }
            };
           
            ///
            /// The index of all found boundaries. Note: for a string with length \a len, 0 and len are always considered
            /// boundaries
            /// 
            typedef std::vector<break_info> index_type;


            ///
            /// Find boundary positions of type \a t for text in range [begin,end) using locale \a loc
            ///
            template<typename CharType>
            index_type map(boundary_type t,CharType const *begin,CharType const *end,std::locale const &loc=std::locale());
            
            ///
            /// Find boundary positions of type \a t for string \a str using locale \a loc
            ///
            template<typename CharType>
            static index_type map(
                            boundary_type t,
                            std::basic_string<CharType> const &str,
                            std::locale const &loc=std::locale())
            {
                return boost::locale::boundary::map<CharType>(t,str.data(),str.data()+str.size(),loc);
            }
            
            template<>
            BOOST_LOCALE_DECL index_type 
            map(boundary_type t,char const *begin,char const *end,std::locale const &loc);

            #ifndef BOOST_NO_STD_WSTRING
            template<>
            BOOST_LOCALE_DECL index_type 
            map(boundary_type t,wchar_t const *begin,wchar_t const *end,std::locale const &loc);
            #endif

            #ifdef BOOST_HAS_CHAR16_T
            template<>
            BOOST_LOCALE_DECL index_type 
            map(boundary_type t,char16_t const *begin,char16_t const *end,std::locale const &loc);
            #endif

            #ifdef BOOST_HAS_CHAR32_T
            template<>
            BOOST_LOCALE_DECL index_type 
            map(boundary_type t,char32_t const *begin,char32_t const *end,std::locale const &loc);
            #endif
            
            namespace details {
                template<typename IteratorType,typename CategoryType = typename std::iterator_traits<IteratorType>::iterator_category>
                struct mapping_traits {
                    typedef typename std::iterator_traits<IteratorType>::value_type char_type;
                    static index_type map(boundary_type t,IteratorType b,IteratorType e,std::locale const &l)
                    {
                        std::basic_string<char_type> str(b,e);
                        return boost::locale::boundary::map(t,str,l);
                    }
                };

                template<typename IteratorType>
                struct mapping_traits<IteratorType,std::random_access_iterator_tag> {
                    typedef typename std::iterator_traits<IteratorType>::value_type char_type;

                    static index_type map(boundary_type t,IteratorType b,IteratorType e,std::locale const &l)
                    {
                        index_type result;
                        if(&*e - &*b == e - b) {
                            index_type tmp=boost::locale::boundary::map(t,&*b,&*e,l);
                            result.swap(tmp);
                        }
                        else{
                            std::basic_string<char_type> str(b,e);
                            index_type tmp=boost::locale::boundary::map(t,str,l);
                            result.swap(tmp);
                        }
                        return result;
                    }
                };

            } // details 

            ///
            /// \brief Class the holds boundary mapping of the text that can be used with iterators
            ///
            /// When the object is created in creates index and provides access to it with iterators.
            /// it is used mostly together with break_iterator and token_iterator
            ///
            /// This class stores iterators to the original text, so you should be careful with iterators
            /// invalidation. The text, iterators pointing to should not change
            ///

            template<typename IteratorType>
            class mapping {
            public:
                typedef typename std::iterator_traits<IteratorType>::value_type char_type;

                ///
                /// Create a mapping of type \a type of the text in range [\a begin, \a end) using locale \a loc
                ///
                mapping(boundary_type type,IteratorType begin,IteratorType end,std::locale const &loc = std::locale())
                {
                    index_type idx=details::mapping_traits<IteratorType>::map(type,begin,end,loc);
                    index_.swap(idx);
                    begin_ = begin;
                    end_ = end;
                }

                ///
                /// Get \a begin iterator used when object was created
                ///
                IteratorType begin() const
                {
                    return begin_;
                }
                ///
                /// Get \a end iterator used when object was created
                ///
                IteratorType end() const
                {
                    return end_;
                }

                ///
                /// Return a const reference to index holding boundaries of the original text
                ///
                index_type const &map() const
                {
                    return index_;
                }
                ///
                /// Return a reference to index holding boundaries of the original text
                ///
                index_type &map()
                {
                    return index_;
                }

            private:
                IteratorType begin_,end_;
                index_type index_;
            };

            ///
            /// \brief break_iterator is bidirectional iterator that returns text boundary positions
            ///
            /// This iterator is used when boundary points are more interesting then text chunks themselves.
            ///
            template<typename IteratorType>
            class break_iterator : public std::iterator<std::bidirectional_iterator_tag,IteratorType> {
            public:
                typedef typename std::iterator_traits<IteratorType>::value_type char_type;
                
                ///
                /// Create iterator instance that can be used for determination of end of text range.
                ///
                /// Note: this iterator instance can't be decremented, it is rather used for boundary condition tests
                ///
                break_iterator() : 
                    map_(0),
                    offset_(0),
                    mask_(0)
                {
                }

                break_iterator(break_iterator<IteratorType> const &other):
                    map_(other.map_),
                    offset_(other.offset_),
                    mask_(other.mask_)
                {
                }
                
                break_iterator const &operator=(break_iterator<IteratorType> const &other)
                {
                    if(this!=&other) {
                        map_ = other.map_;
                        offset_ = other.offset_;
                        mask_ = other.mask_;
                    }
                    return *this;
                }

                ~break_iterator()
                {
                }

                ///
                /// Set break iterator to new position \a p. 
                ///
                /// \a p should be in range of [\a begin, \a end] of the original text.
                ///
                /// If \a p points to actual boundary point, break iterator would point to this point, otherwise,
                /// it would point to the proceeding boundary point.
                ///
                /// Note: break iterator should be created and mapping should be assigned to it.
                ///
                /// \code 
                ///    break_iterator<char const *>  begin(map),end;
                ///    begin=some_point_in_text; // Ok
                ///    end=some_point_in_text; // Wrong!
                /// \endcode
                ///
                break_iterator const &operator=(IteratorType p)
                {
                    at_most(p);
                    return *this;
                }

                ///
                /// Create a break iterator pointing to the beginning of the range.
                ///
                /// \a map -- is the mapping used with iterator. Should be valid all the time iterator is used.
                ///
                /// \a mask -- mask of flags for which boundary points are counted. If \a mask is 0, all boundary
                /// points are used, otherwise, only points giving break_info::brk & mask !=0 are used, others are skipped.
                ///
                /// For example.
                ///
                /// \code
                ///   boundary::mapping<char *> a_map(boundary::line,begin,end);
                ///   boundart::break_iteraotr<char *> p(a_map,boundary::soft),e;
                //// \endcode
                /// 
                /// Would create an iterator p that would iterate only over soft line break points.
                ///
                break_iterator(mapping<IteratorType> const &map,unsigned mask = 0) :
                    map_(&map),
                    offset_(0),
                    mask_(mask)
                {
                }

                ///
                /// Compare two iterators, note. Empty iterator and iterator that points to the end
                /// of the range are considered even semantically they are not same.
                ///
                bool operator==(break_iterator<IteratorType> const &other) const
                {
                    return  (map_ == other.map_ && offset_==offset_)
                            || (at_end() && other.at_end());
                }

                ///
                /// Opposite of ==
                ///
                bool operator!=(break_iterator<IteratorType> const &other) const
                {
                    return !(*this==other);
                }

                ///
                /// Return the underlying iterator to the boundary point. Note: mapping \a begin and \a end 
                /// are valid points.
                ///
                /// Do not use this operator one empty iterators
                ///
                IteratorType operator*() const
                {
                    return map_->begin() + map_->map()[offset_].offset;
                }
                
                break_iterator &operator++() 
                {
                    next();
                    return *this;
                }
                
                break_iterator &operator--() 
                {
                    prev();
                    return *this;
                }
                
                break_iterator operator++(int unused) 
                {
                    break_iterator tmp(*this);
                    next();
                    return tmp;
                }

                break_iterator operator--(int unused) 
                {
                    break_iterator tmp(*this);
                    prev();
                    return tmp;
                }


            private:
                void at_most(IteratorType p)
                {
                    unsigned diff =  p - map_->begin();
                    index_type::iterator ptr = std::lower_bound(map_->map().begin(),map_->map().end(),break_info(diff));
                    if(ptr==map_->map().end())
                        offset_=map_->map().size()-1;
                    else
                        offset_=ptr - map_->map().begin();
                    if(mask_==0)
                        return;
                    while(offset_ > 0 && (map_->map()[offset_].brk & mask_) == 0)
                        offset_--;
                }
                bool at_end() const
                {
                    return !map_ || offset_ >= map_->map().size();
                }
                void next()
                {
                    do {
                        offset_++;
                    }while(!at_end() && mask_ != 0 && ((map_->map()[offset_].brk & mask_) == 0));
                }
                void prev()
                {
                    do {
                        offset_--;
                    }while(offset_ >0 && mask_ != 0 && ((map_->map()[offset_].brk & mask_) == 0));
                }

                mapping<IteratorType> const * map_;
                size_t offset_;
                unsigned mask_;
            };

            ///
            /// \brief token iterator is an iterator that returns text chunks between boundary positions
            ///
            /// It is similar to break iterator, but it rather "points" to string then iterator. It is used when we are more
            /// interested in text chunks themselves then boundary points.
            ///
            
            template<typename IteratorType,typename ValueType = std::basic_string<typename std::iterator_traits<IteratorType>::value_type> >
            class token_iterator : public std::iterator<std::bidirectional_iterator_tag,ValueType> {
            public:
                typedef typename std::iterator_traits<IteratorType>::value_type char_type;
                                
                token_iterator() : 
                    map_(0),
                    offset_(0),
                    mask_(0)
                {
                }

                token_iterator(token_iterator<IteratorType,ValueType> const &other):
                    map_(other.map_),
                    offset_(other.offset_),
                    mask_(other.mask_)
                {
                }
                
                token_iterator const &operator=(token_iterator<IteratorType,ValueType> const &other)
                {
                    if(this!=&other) {
                        map_ = other.map_;
                        offset_ = other.offset_;
                        mask_ = other.mask_;
                    }
                    return *this;
                }

                ~token_iterator()
                {
                }

                token_iterator const &operator=(IteratorType p)
                {
                    at_most(p);
                    return *this;
                }
                token_iterator(mapping<IteratorType> const &map,unsigned mask = 0) :
                    map_(&map),
                    offset_(0),
                    mask_(mask)
                {
                    if(mask!=0 && (map_->map()[0].next & mask)==0)
                        next();
                }

                bool operator==(token_iterator<IteratorType,ValueType> const &other) const
                {
                    return  (map_ == other.map_ && offset_==offset_)
                            || (at_end() && other.at_end());
                }

                bool operator!=(token_iterator<IteratorType,ValueType> const &other) const
                {
                    return !(*this==other);
                }

                ValueType operator*() const
                {
                    IteratorType ob=map_->begin() + map_->map()[offset_].offset;
                    IteratorType oe=map_->begin() + map_->map()[offset_+1].offset;
                    return ValueType(ob,oe);
                }
                
                token_iterator &operator++() 
                {
                    next();
                    return *this;
                }
                
                token_iterator &operator--() 
                {
                    prev();
                    return *this;
                }
                
                token_iterator operator++(int unused) 
                {
                    token_iterator<IteratorType,ValueType> tmp(*this);
                    next();
                    return tmp;
                }

                token_iterator operator--(int unused) 
                {
                    token_iterator<IteratorType,ValueType> tmp(*this);
                    prev();
                    return tmp;
                }


            private:
                void at_most(IteratorType p)
                {
                    unsigned diff =  p - map_->begin();
                    index_type::iterator ptr = std::lower_bound(map_->map().begin(),map_->map().end(),break_info(diff));
                    if(ptr==map_->map().end())
                        offset_=map_->map().size()-1;
                    else
                        offset_=ptr - map_->map().begin();
                    if(mask_==0)
                        return;
                    while(offset_ > 0 && (map_->map()[offset_].next & mask_) == 0)
                        offset_--;
                }
                bool at_end() const
                {
                    return !map_ || offset_ >= map_->map().size()-1;
                }
                void next()
                {
                    do {
                        offset_++;
                    }while(!at_end() && mask_ != 0 && ((map_->map()[offset_].next & mask_) == 0));
                }
                void prev()
                {
                    do {
                        offset_--;
                    }while(offset_ >0 && mask_ != 0 && ((map_->map()[offset_].next & mask_) == 0));
                }

                mapping<IteratorType> const * map_;
                size_t offset_;
                unsigned mask_;
            };
            
            
        } // boundary
    } // locale
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
