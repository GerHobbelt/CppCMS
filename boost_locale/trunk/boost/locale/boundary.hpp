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

#include <locale>
#include <vector>
#include <iterator>

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
                ideo    = 1 << 3    ///< Word that contains ideographic characters
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
            BOOST_LOCALE_DECL boundary::index_type 
            boundary::map(boundary::boundary_type t,char const *begin,char const *end,std::locale const &loc);

            #ifndef BOOST_NO_STD_WSTRING
            template<>
            BOOST_LOCALE_DECL boundary::index_type 
            boundary::map(boundary::boundary_type t,wchar_t const *begin,wchar_t const *end,std::locale const &loc);
            #endif

            #ifdef BOOST_HAS_CHAR16_T
            template<>
            BOOST_LOCALE_DECL boundary::index_type 
            boundary::map(boundary::boundary_type t,char16_t const *begin,char16_t const *end,std::locale const &loc);
            #endif

            #ifdef BOOST_HAS_CHAR32_T
            template<>
            BOOST_LOCALE_DECL boundary::index_type 
            boundary::map(boundary::boundary_type t,char32_t const *begin,char32_t const *end,std::locale const &loc);
            #endif
            namespace details {
                template<typename IteratorType>
                struct mapping_traits {
                    typedef typename std::iterator_traits<IteratorType>::value_type char_type;
                    //
                    // Most Generic not efficient implementation
                    //
                    static index_type map(boundary_type t,IteratorType b,IteratorType e,std::locale const &l)
                    {
                        std::basic_string<char_type> str(b,e);
                        return boost::locale::boundary::map(t,str,l);
                    }
                };

                template<typename CharType>
                struct mapping_traits<CharType const *> {
                    static index_type map(boundary_type t,CharType const *b,CharType const *e,std::locale const &l)
                    {
                        return boost::locale::boundary::map(t,b,e,l);
                    }
                };
                
                template<typename CharType>
                struct mapping_traits<CharType *> {
                    static index_type map(boundary_type t,CharType *b,CharType *e,std::locale const &l)
                    {
                        return boost::locale::boundary::map(t,b,e,l);
                    }
                };
                template<typename CharType,typename Traits,typename Alloc>
                struct mapping_traits<typename std::basic_string<CharType,Traits,Alloc>::const_iterator> {
                    typedef typename std::basic_string<CharType>::const_iterator iterator_type;
                    static index_type map(boundary_type t,iterator_type b,iterator_type e,std::locale const &l)
                    {
                        if(&*e-&*b == e-b) // Check continuos
                            return boost::locale::boundary::map(t,&*b,&*e,l);
                        else {
                            std::basic_string<CharType> str(b,e);
                            return boost::locale::boundary::map(t,str,l);
                        }
                    }
                };

                template<typename CharType,typename Traits,typename Alloc>
                struct mapping_traits<typename std::basic_string<CharType,Traits,Alloc>::iterator> {
                    typedef typename std::basic_string<CharType>::iterator iterator_type;
                    static index_type map(boundary_type t,iterator_type b,iterator_type e,std::locale const &l)
                    {
                        if(&*e-&*b == e-b) // Check continuos
                            return boost::locale::boundary::map(t,&*b,&*e,l);
                        else {
                            std::basic_string<CharType> str(b,e);
                            return boost::locale::boundary::map(t,str,l);
                        }
                    }
                };
 

            } // details 


            template<typename IteratorType>
            class mapping {
            public:
                typedef typename std::iterator_traits<IteratorType>::value_type char_type;
                mapping(boundary_type type,IteratorType begin,IteratorType end,std::locale const &loc = std::locale())
                {
                    index_type idx=details::mapping_traits<IteratorType>::map(type,begin,end,loc);
                    index_.swap(idx);
                    begin_ = begin;
                    end_ = end;
                }

                IteratorType begin() const
                {
                    return begin_;
                }
                IteratorType end() const
                {
                    return end_;
                }
                index_type const &map() const
                {
                    return index_;
                }
                index_type &map()
                {
                    return index_;
                }

            private:
                IteratorType begin_,end_;
                index_type index_;
            };

            template<typename IteratorType>
            class break_iterator : public std::iterator<std::bidirectional_iterator_tag,IteratorType> {
            public:
                typedef typename std::iterator_traits<IteratorType>::value_type char_type;
                
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
                void at_least(IteratorType p)
                {
                    unsigned diff =  p - map_->begin();
                    index_type::iterator ptr = std::upper_bound(map_->map().begin(),map_->map().end(),break_info(diff));
                    if(ptr==map_->map().end())
                        offset_=map_->map().size()-1;
                    else
                        offset_=ptr - map_->map().begin();
                    if(mask_==0)
                        return;
                    while(offset_ > 0 && (map_->map()[offset_].brk & mask_) == 0)
                        offset_--;
                }
                break_iterator const &operator=(IteratorType p)
                {
                    at_most(p);
                    return *this;
                }
                break_iterator(mapping<IteratorType> const &map,unsigned mask = 0) :
                    map_(&map),
                    offset_(0),
                    mask_(0)
                {
                }

                bool operator==(break_iterator<IteratorType> const &other) const
                {
                    return  map_ == other.map_ && offset_==offset_ 
                            || (at_end() && other.at_end());
                }

                bool operator!=(break_iterator<IteratorType> const &other) const
                {
                    return !(*this==other);
                }

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
                bool at_end() const
                {
                    return !map_ || offset_ >= map_->map().size();
                }
                void next()
                {
                    do {
                        offset_++;
                    }while(!at_end() && mask_ != 0 && (map_->map()[offset_].brk & mask_ == 0));
                }
                void prev()
                {
                    do {
                        offset_--;
                    }while(offset_ >0 && mask_ != 0 && (map_->map()[offset_].brk & mask_ == 0));
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
