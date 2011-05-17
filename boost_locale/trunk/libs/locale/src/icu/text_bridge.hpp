//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_SRC_LOCALE_TEXT_BRIDGE_HPP
#define BOOST_SRC_LOCALE_TEXT_BRIDGE_HPP
#include <unicode/chariter.h>
#include <unicode/ustring.h>

#include <boost/locale/text.hpp>
#include <typeinfo>

namespace boost {
namespace locale {
namespace impl_icu {

    struct latin1_converter {
    public:
        static const bool bmp_only = true;
        UChar operator()(char v) const
        {
            return static_cast<unsigned char>(v);
        }
    };

    struct utf16_converter {
    public:
        static const bool bmp_only = false;
        UChar operator()(uint16_t v) const
        {
            return v;
        }
    };

    struct simple_encoding_converter {
    public:
        static const bool bmp_only = true;
        simple_encoding_converter(UChar const *t) : table_(t) {}
        UChar operator()(char v) const
        {
            return table_[static_cast<unsigned char>(v)];
        }
    private:
        UChar const *table_;
    };


    static char utf_class_id;
    
    template<typename IteratorType>
    class basic_utf_char_iter : public icu::CharacterIterator {
    public:
        basic_utf_char_iter(IteratorType b,IteratorType e) :
            it(b),
            it_begin(b),
            it_end(e)
        {
        }
        
        IteratorType const &base_iterator() const
        {
            return it;
        }
        
        IteratorType &base_iterator()
        {
            return it;
        }

    protected:
        IteratorType it;
        IteratorType it_begin;
        IteratorType it_end;
    };

    template<typename RealType,typename IteratorType>
    class utf_char_iter_base : public basic_utf_char_iter<IteratorType> {
    public:
        utf_char_iter_base(IteratorType b,IteratorType e) :
            basic_utf_char_iter<IteratorType>(b,e)
        {
            this->begin = this->pos = this->end = this->textLength = 0;
            index = 0;
            size = 0;
        }
        virtual UClassID getDynamicClassID() const
        {
            return (UClassID)(&utf_class_id);
        }
        UBool operator==(ForwardCharacterIterator const &other_in) const
        {
            if(typeid(*this)!=typeid(other_in)) {
                return FALSE;
            }
            utf_char_iter_base const &other = static_cast<utf_char_iter_base const &>(other_in);
            return this->it == other.it && index == other.index;
        }
        int32_t hashCode() const
        {
            return this->pos;
        }
        UChar next()
        {
            if(size == 0)
                return icu::CharacterIterator::DONE;
            if(size == 2 && index==0) {
                this->pos++;
                index++;
                return value[1];
            }
            if(this->it == this->it_end)
                return icu::CharacterIterator::DONE;
            this->pos++;
            index = 0;
            real().extract_next();
            return current();
        }
        UChar32 next32()
        {
            if(size == 0)
                return icu::CharacterIterator::DONE;
            if(size == 2)
            {
                if(index==0) 
                    this->pos+=2;
                else // index == 1
                    this->pos++;
            }
            else {
                this->pos++;
            }
            if(this->it == this->it_end)
                return icu::CharacterIterator::DONE;
            index = 0;
            real().extract_next();
            return current32();
        }
        
        UChar nextPostInc() 
        {
            if(size == 0)
                return icu::CharacterIterator::DONE;
            if(size == 1) {
                UChar result = value[0];
                this->pos++;
                real().extract_next();
                index = 0;
                return result;
            }
            else { // size == 2
                UChar result = value[index];
                this->pos++;
                if(index == 0) {
                    index = 1;
                }
                else {
                    real().extract_next();
                    index = 0;
                }
                return result;
            }
        }
        UChar32 next32PostInc()
        {
            if(size == 0)
                return icu::CharacterIterator::DONE;
            UChar32 res;
            if(size == 2) {
                res = U16_GET_SUPPLEMENTARY(value[0],value[1]);
                if(index == 0) {
                    this->pos+=2;
                    real().extract_next();
                }
                else {
                    this->pos+=1;
                    real().extract_next();
                }
            }
            else {
                res = value[0];
                this->pos++;
                real().extract_next();
            }
            return res;
        }
        UBool hasNext() 
        {
            return this->it!=this->it_end || index < size;
        }
        icu::CharacterIterator *clone() const
        {
            return new RealType(real());
        }
        UChar first() 
        {
            this->pos = 0;
            index = 0;
            this->it = this->it_begin;
            real().extract_curr();
            return current();
        }
        UChar32 first32()
        {
            this->pos = 0;
            index = 0;
            this->it = this->it_begin;
            real().extract_curr();
            return current32();
        }
        UChar current() const
        {
            if(size == 0)
                return icu::CharacterIterator::DONE;
            else
                return value[index];
        }
        UChar32 current32() const
        {
            if(size == 0)
                return icu::CharacterIterator::DONE;
            else if(size == 1)
                return value[0];
            else {
                return U16_GET_SUPPLEMENTARY(value[0],value[1]);
            }
        }
        
        UChar previous() 
        {
            if(this->pos == 0)
                return icu::CharacterIterator::DONE;
            if(size == 2 && index==1) {
                index = 0;
                this->pos --;
                return value[0];
            }
            this->pos--;
            real().extract_prev();
            if(size == 2) {
                index=1;
                return value[1];
            }
            else {
               return value[0];
            }
        }
        UChar32 previous32() 
        {
            if(this->pos == 0)
                return icu::CharacterIterator::DONE;
            if(size == 2 && index==1) {
                index = 0;
                this->pos --;
            }
            this->pos--;
            
            real().extract_prev();
            
            if(size == 2) {
                index=0;
                this->pos--;
            }

            return current32();
        }
        UBool hasPrevious()
        {
            return this->pos > 0;
        }
        UChar setIndex(int32_t idx)
        {
            if(idx > this->pos /2)
                move(idx - this->pos,icu::CharacterIterator::kCurrent);
            else
                move(idx,icu::CharacterIterator::kStart);
            return current();
        }
        UChar32 setIndex32(int32_t idx)
        {
            if(idx > this->pos /2)
                move(idx - this->pos,icu::CharacterIterator::kCurrent);
            else
                move(idx,icu::CharacterIterator::kStart);
            if(size == 2 && index == 1) {
                index = 0;
                this->pos --;
            }
            return current32();
        }
        void getText(icu::UnicodeString &out)
        {
            int32_t current = this->pos;
            first();
            UChar c;
            while((c=nextPostInc())!=icu::CharacterIterator::DONE) {
                out+=c;
            }
            setIndex(current);
        }
        UChar last()
        {
            real().go_to_end();
            return previous();
        }
        UChar32 last32()
        {
            real().go_to_end();
            return previous32();
        }
        int32_t move32(int32_t new_pos,icu::CharacterIterator::EOrigin o)
        {
            if(o==icu::CharacterIterator::kStart) {
                this->it = this->it_begin;
                this->pos = 0;
                index = 0;
            }
            else if(o == icu::CharacterIterator::kEnd) {
                real().go_to_end();
            }
            real().advance32(new_pos);
            return this->pos;
        }
        int32_t move(int32_t new_pos,icu::CharacterIterator::EOrigin o)
        {
            if(o==icu::CharacterIterator::kStart) {
                this->it = this->it_begin;
                this->pos = 0;
                index = 0;
            }
            else if(o == icu::CharacterIterator::kEnd) {
                real().go_to_end();
            }
            real().advance(new_pos);
            return this->pos;
        }

    protected:
        RealType &real()
        {
            return static_cast<RealType &>(*this);
        }
        RealType const &real() const
        {
            return static_cast<RealType const &>(*this);
        }

        int index;
        int size;
        UChar value[2];
    };

    template<typename IteratorType,typename Converter>
    class utf16_char_iter : public utf_char_iter_base<
                                        utf16_char_iter<IteratorType,Converter>,
                                        IteratorType
                                    >
    {
    public:
        typedef typename std::iterator_traits<IteratorType>::value_type char_type;

        utf16_char_iter(IteratorType b,IteratorType e,Converter c) :
            utf_char_iter_base<utf16_char_iter<IteratorType,Converter>,IteratorType>(b,e),
            real_end_pos(-1),
            converter(c)
        {
            extract_curr();
        }
        void advance32(int32_t offset)
        {
            if(Converter::bmp_only) {
                advance(offset);
                return;
            }

            extract_curr();

            if(this->index==1 && this->size==2) {
                this->pos--;
                this->index--;
            }

            while(offset > 0 && this->hasNext()) {
                this->next32();
                offset--;
            }
            while(offset < 0 && this->pos > 0) {
                this->previous32();
                offset++;
            }
        }
        
        void advance(int32_t offset)
        {
            if(!Converter::bmp_only && this->size == 2 && this->index == 1) {
                this->index--;
                this->pos--;
                offset++;
            }
            
            this->pos+=advance_in_range(this->it,offset,this->it_begin,this->it_end);

            if(this->it == this->it_end) {
                this->size = 0;
                return;
            }
            UChar c=converter(*this->it);
            if(Converter::bmp_only) {
                this->size = 1;
                this->value[0]=c;
            }
            else {
                if(U16_IS_TRAIL(c) && this->it!=this->it_begin){
                    this->value[1]=c;
                    --this->it;
                    this->value[0]=converter(*this->it);
                    this->index = 1;
                    this->size = 2;
                }
                else if(U16_IS_LEAD(c)) {
                    this->index = 0;
                    extract_curr();
                }
                else {
                    this->value[0]=c;
                    this->size = 1;
                    this->index = 0;
                }
            }
        }
        void go_to_end()
        {
            if(real_end_pos!=-1) {
                this->it = this->it_end;
                this->pos = real_end_pos;
                this->index = 0;
                this->size = 0;
            }
            else {
                this->pos += std::distance(this->it,this->it_end);
                real_end_pos = this->pos;
                this->it = this->it_end;
                this->index = 0;
                this->size = 0;
                this->end = this->textLength = this->pos;
            }
        }
        void extract_curr()
        {
            if(Converter::bmp_only) {
                if(this->it!=this->it_end) {
                    this->value[0] = converter(*this->it);
                    this->size = 1;
                }
                else {
                    this->size = 0;
                }
            }
            else {
                if(this->it==this->it_end) {
                    this->size = 0;
                }
                else {
                    this->value[0] = converter(*this->it);
                    this->size = 1;
                    if(U16_IS_LEAD(this->value[0])) {
                        ++this->it;
                        if(this->it!=this->it_end) {
                            this->value[1] = converter(*this->it);
                            this->size = 2;
                        }
                        --this->it;
                    }
                }
            }
        }
        void extract_next()
        {
            if(this->it!=this->it_end) {
                std::advance(this->it,this->size);
                extract_curr();
            }
        }
        void extract_prev()
        {
            if(this->it!=this->it_begin) {
                --this->it;
                if(Converter::bmp_only) {
                    this->value[0]=converter(*this->it);
                    this->size = 1;
                }
                else {
                    UChar c=converter(*this->it);
                    if(this->it!=this->it_begin && U16_IS_TRAIL(c)) {
                        --this->it;
                        this->value[0]=converter(*this->it);
                        this->value[1]=c;
                        this->size = 2;
                    }
                    else {
                        this->value[0] = c;
                        this->size = 1;
                    }
                }
            }
        }
    private:
        int32_t real_end_pos;
        Converter converter;
    };


    template<typename IteratorType>
    class utf8_char_iter : public utf_char_iter_base<utf8_char_iter<IteratorType>,IteratorType>
    {
    public:
        typedef utf8_char_iter<IteratorType> this_type;

        utf8_char_iter(IteratorType begin,IteratorType end) :
            utf_char_iter_base<utf8_char_iter<IteratorType>,IteratorType>(begin,end),
            real_end_pos(-1),
            curr_len(0)
        {
            extract_curr();
        }
        
        void advance32(int32_t offset)
        {
            /// FIXME
            extract_curr();     
            while(offset > 0 && this_type::hasNext()){
                this_type::next32();
                offset--;
            }
            while(offset < 0 && this->pos > 0) {
                this_type::previous32();
                offset++;
            }
        }

        void advance(int32_t offset)
        {
            /// FIXME
            extract_curr();     
            while(offset > 0 && this_type::hasNext()){
                this_type::next();
                offset--;
            }
            while(offset < 0 && this->pos > 0) {
                this_type::previous();
                offset++;
            }
        }
        void go_to_end()
        {
            if(real_end_pos!=-1) {
                this->it = this->it_end;
                this->pos = real_end_pos;
                this->index = 0;
                this->size = 0;
            }
            else {
                // FIXME
                while(this_type::next()!=icu::CharacterIterator::DONE)
                    ;
                real_end_pos = this->pos;
                this->end = this->textLength = this->pos;
            }
        }
        void extract_curr()
        {
            if(this->it==this->it_end) {
                this->size =0;
                curr_len = 0;
                return;
            }
            char lead = *this->it;
            if(U8_IS_SINGLE(lead)) {
                this->size = 1;
                this->value[0]=lead;
                curr_len = 1;
            }
            else {
                char buf[4] = {lead};
                int lead_len = U8_COUNT_TRAIL_BYTES(lead) + 1;
                curr_len = copy_range(this->it,this->it_end,buf,buf+lead_len) - buf;
                int i = 0;
                UChar32 uc = 0;
                U8_NEXT_UNSAFE(buf,i,uc);
                if(U_IS_SUPPLEMENTARY(uc)) {
                    this->size = 2;
                    this->value[0]=U16_LEAD(uc);
                    this->value[1]=U16_TRAIL(uc);
                }
                else {
                    this->size = 1;
                    this->value[0] = uc;
                }
            }
        }
        void extract_next()
        {
            std::advance(this->it,curr_len);
            extract_curr();
        }
        void extract_prev()
        {
            if(this->it == this->it_begin) {
                return;
            }
            char buf[4];
            char *b = buf+4;
            char *e = buf+4;
            char c;
            do {
                c=*--b = *--(this->it);
            }while(U8_IS_TRAIL(c) && this->it!=this->it_begin && b!=buf);
            
            curr_len = e - b;
            int i = 0;
            UChar32 uc=0;
            
            U8_NEXT(b,i,curr_len,uc);

            if(U_IS_SUPPLEMENTARY(uc)) {
                this->size = 2;
                this->value[0]=U16_LEAD(uc);
                this->value[1]=U16_TRAIL(uc);
            }
            else {
                this->size = 1;
                this->value[0] = uc;
            }
        }
    private:
        int32_t real_end_pos;
        int curr_len;
    };

    template<typename IteratorType>
    class utf32_char_iter : public utf_char_iter_base<utf32_char_iter<IteratorType>,IteratorType>
    {
    public:
        typedef utf32_char_iter<IteratorType> this_type;

        utf32_char_iter(IteratorType begin,IteratorType end) :
            utf_char_iter_base<utf32_char_iter<IteratorType>,IteratorType>(begin,end),
            real_end_pos(-1)
        {
            extract_curr();
        }
        
        void advance32(int32_t offset)
        {
            /// FIXME
            extract_curr();     
            while(offset > 0 && this_type::hasNext()){
                this_type::next32();
                offset--;
            }
            while(offset < 0 && this->pos > 0) {
                this_type::previous32();
                offset++;
            }
        }

        void advance(int32_t offset)
        {
            /// FIXME
            extract_curr();     
            while(offset > 0 && this_type::hasNext()){
                this_type::next();
                offset--;
            }
            while(offset < 0 && this->pos > 0) {
                this_type::previous();
                offset++;
            }
        }
        void go_to_end()
        {
            if(real_end_pos!=-1) {
                this->it = this->it_end;
                this->pos = real_end_pos;
                this->index = 0;
                this->size = 0;
            }
            else {
                // FIXME
                while(this_type::next()!=icu::CharacterIterator::DONE)
                    ;
                real_end_pos = this->pos;
                this->end = this->textLength = this->pos;
            }
        }
        void extract_curr()
        {
            if(this->it==this->it_end) {
                this->size =0;
                return;
            }
            UChar32 c = static_cast<UChar32>(*this->it);
            if(U_IS_BMP(c)) {
                this->size = 1;
                this->value[0]=static_cast<UChar>(c);
            }
            else {
                this->size = 2;
                this->value[0]=U16_LEAD(c);
                this->value[1]=U16_TRAIL(c);
            }
        }
        void extract_next()
        {
            if(this->it!=this->it_end) {
                ++this->it;
            }
            extract_curr();
        }
        void extract_prev()
        {
            if(this->it != this->it_begin) {
                --this->it;
            }
            extract_curr();
        }
    private:
        int32_t real_end_pos;
    };

    template<typename Iterator>
    basic_utf_char_iter<Iterator> *utf8_iterator(Iterator b,Iterator e)
    {
        return new utf8_char_iter<Iterator>(b,e);
    }
    
    template<typename Iterator>
    basic_utf_char_iter<Iterator> *latin1_iterator(Iterator b,Iterator e)
    {
        return new utf16_char_iter<Iterator,latin1_converter>(b,e,latin1_converter());
    }
    
    template<typename Iterator>
    basic_utf_char_iter<Iterator> *utf16_iterator(Iterator b,Iterator e)
    {
        return new utf16_char_iter<Iterator,utf16_converter>(b,e,utf16_converter());
    }
    
    template<typename Iterator>
    basic_utf_char_iter<Iterator> *utf32_iterator(Iterator b,Iterator e)
    {
        return new utf32_char_iter<Iterator>(b,e);
    }




} /// impl_icu
} //  locale
} // boost

#endif


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
