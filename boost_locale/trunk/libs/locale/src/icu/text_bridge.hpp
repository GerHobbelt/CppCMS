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
#include "uconv.hpp"

#include <boost/locale/text.hpp>
#include <boost/shared_ptr.hpp>
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

    struct sbcs_converter {
    public:
        static const bool bmp_only = true;
        sbcs_converter(UChar const *t) : table_(t) {}
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
        basic_utf_char_iter()
        {
        }

        virtual IteratorType base_iterator() const  = 0;

    };

    template<typename RealType,typename IteratorType,typename InputIteratorType = IteratorType>
    class utf_char_iter_base : public basic_utf_char_iter<InputIteratorType> {
    public:
        typedef typename std::iterator_traits<IteratorType>::value_type char_type;
        utf_char_iter_base(IteratorType b,IteratorType e) :
            it(b),
            it_begin(b),
            it_end(e)
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
            return it == other.it && index == other.index;
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
            if(it == it_end)
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
            if(it == it_end)
                return icu::CharacterIterator::DONE;
            index = 0;
            real().extract_next();
            return current32();
        }
        
        UChar nextPostInc() 
        {
            switch(size) {
            case 0:
                return icu::CharacterIterator::DONE;
            case 1:
                {
                    UChar result = value[0];
                    this->pos++;
                    real().extract_next();
                    index = 0;
                    return result;
                }
            case 2:
            default:
                {
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
            return it!=it_end || index < size;
        }
        UChar first() 
        {
            this->pos = 0;
            index = 0;
            it = it_begin;
            real().extract_curr();
            return current();
        }
        UChar32 first32()
        {
            this->pos = 0;
            index = 0;
            it = it_begin;
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
                it = it_begin;
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
                it = it_begin;
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

        IteratorType it;
        IteratorType it_begin;
        IteratorType it_end;

    };

    template<typename IteratorType,typename Converter,typename InputIterator = IteratorType>
    class base_utf16_char_iter : public utf_char_iter_base<
                                            base_utf16_char_iter<IteratorType,Converter,InputIterator>,
                                            IteratorType,
                                            InputIterator
                                        >
    {
    public:
        typedef typename std::iterator_traits<IteratorType>::value_type char_type;

        base_utf16_char_iter(IteratorType b,IteratorType e,Converter c) :
            utf_char_iter_base<
                    base_utf16_char_iter<IteratorType,Converter,InputIterator>,
                    IteratorType,
                    InputIterator
                >
                (b,e),
            converter(c)
        {
            this->end = this->textLength = std::distance(b,e);
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
            this->it = this->it_end;
            this->pos = this->end;
            this->index = 0;
            this->size = 0;
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
    protected:
        Converter converter;
    };

    template<typename IteratorType,typename Converter>
    class utf16_char_iter : public base_utf16_char_iter<IteratorType,Converter>
    {
    public:
        utf16_char_iter(IteratorType b,IteratorType e,Converter c) :
            base_utf16_char_iter<IteratorType,Converter>(b,e,c)
        {
        }
        
        icu::CharacterIterator *clone() const
        {
            return new utf16_char_iter(*this);
        }
        virtual IteratorType base_iterator() const
        {
            return this->it;
        }

    };

    template<typename IteratorType,typename Converter>
    class mbcs_char_iter : public base_utf16_char_iter<UChar const *,Converter,IteratorType>
    {
    public:
        mbcs_char_iter(Converter const &c) :
            base_utf16_char_iter<UChar const *,Converter,IteratorType>(c.begin(),c.end(),c)
        {
        }
        
        icu::CharacterIterator *clone() const
        {
            return new mbcs_char_iter(*this);
        }
 
        virtual IteratorType base_iterator() const
        {
            return this->converter.iterator_for_offset(this->it - this->it_begin);
        }
    };

    template<typename InputIterator,bool BmpOnly = false>
    class mbcs_converter
    {
    public:
        static const bool bmp_only = BmpOnly;
        mbcs_converter(InputIterator f,std::vector<UChar> &data,std::vector<size_t> &offsets) :
            first_iter_(f)
        {
            data_.reset(new std::vector<UChar>());
            data_->swap(data);
            offsets_.reset(new std::vector<size_t>());
            offsets_->swap(offsets);
        }
        UChar operator()(UChar c) const
        {
            return c;
        }
        InputIterator iterator_for_offset(size_t off) const
        {
            InputIterator tmp = first_iter_;
            std::advance(tmp,(*offsets_)[off]);
            return tmp;
        }
        UChar const *begin() const
        {
            static UChar valid_place;
            if(data_->empty())
                return &valid_place;
            else
                return &data_->front();
        }
        UChar const *end() const
        {
            return begin() + data_->size();
        }

    private:
        InputIterator first_iter_;
        boost::shared_ptr<std::vector<UChar> > data_;
        boost::shared_ptr<std::vector<size_t> > offsets_;

    };




    template<typename IteratorType>
    class utf8_char_iter : public utf_char_iter_base<utf8_char_iter<IteratorType>,IteratorType>
    {
    public:
        typedef utf8_char_iter<IteratorType> this_type;

        utf8_char_iter(IteratorType begin,IteratorType end) :
            utf_char_iter_base<utf8_char_iter<IteratorType>,IteratorType>(begin,end),
            curr_len(0)
        {
            for(IteratorType it=begin;it!=end;++it) {
                char c=*it;
                if(U8_IS_SINGLE(c))
                    this->end++;
                else if(U8_IS_LEAD(c)) {
                    int n = U8_COUNT_TRAIL_BYTES(c);
                    if(n == 3)
                        this->end+=2;
                    else
                        this->end++;
                }
            }
            this->textLength = this->end;
            extract_curr();
        }
        
        icu::CharacterIterator *clone() const
        {
            return new utf8_char_iter(*this);
        }

        virtual IteratorType base_iterator() const
        {
            return this->it;
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
            /*extract_curr();     
            while(offset > 0 && this_type::hasNext()){
                this_type::next();
                offset--;
            }
            while(offset < 0 && this->pos > 0) {
                this_type::previous();
                offset++;
            }*/
            if(this->size == 2 && this->index == 1) {
                offset++;
                this->pos--;
                this->index=0;
            }
            if(offset + this->pos > this->end) {
                offset = this->end - this->pos;
            }
            if(offset > 0) {
                int trail=curr_len;
                int count=0;
                while(offset > 0 && this->it!=this->it_end) {
                    char lead=*this->it;
                    trail = U8_COUNT_TRAIL_BYTES(lead) + 1;
                    if(trail == 4) {
                        this->pos+=2;
                        offset-=2;
                    }
                    else {
                        this->pos++;
                        offset--;
                    }
                    while(trail > 0 && this->it!=this->it_end) {
                        ++this->it;
                        trail--;
                        count++;
                    }
                    curr_len = trail - count;
                }
                if(offset < 0) {
                    extract_curr();
                    this_type::previous();
                }
                else {
                    extract_curr();
                    this->index = 0;
                }
            }
            else if(offset == 0) {
                extract_curr();
            }
            else { // offset < 0

                while(offset < 0 && this->pos > 0) {
                    this_type::previous();
                    offset++;
                }
                extract_curr();
                #if 0
                // FIXME start
                if(this->pos + offset < 0) {
                    this->pos = 0;
                    this->it = this->it_begin;
                    this->index = 0;
                    extract_curr();
                    return;
                }
                while(offset < 0) {
                    char c;
                    do {
                        --this->it;
                        c=*this->it;
                    }while(U8_IS_TRAIL(c));
                    curr_len = U8_COUNT_TRAIL_BYTES(c) + 1;
                    if(curr_len == 4) {
                        offset +=2;
                        this->size = 2;
                    }
                    else {
                        offset++;
                        this->size = 1;
                    }
                }
                this->index = 0;
                extract_curr();
                if(offset > 0) {
                    this_type::next();
                }
                // FIXME end
                #endif
            }
        }
        void go_to_end()
        {
            this->it = this->it_end;
            this->pos = this->end;
            this->index = 0;
            this->size = 0;
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
                curr_len = 1;
                do{
                    ++this->it;
                    if(this->it == this->it_end)
                        break;
                    buf[curr_len] = *this->it;
                    curr_len ++;
                } while(curr_len < lead_len);
                std::advance(this->it,-curr_len + 1);
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
        int curr_len;
    };

    template<typename IteratorType>
    class utf32_char_iter : public utf_char_iter_base<utf32_char_iter<IteratorType>,IteratorType>
    {
    public:
        typedef utf32_char_iter<IteratorType> this_type;

        utf32_char_iter(IteratorType begin,IteratorType end) :
            utf_char_iter_base<utf32_char_iter<IteratorType>,IteratorType>(begin,end)
        {
            for(IteratorType it=begin;it!=end;++it) {
                UChar32 c=*it;
                if(U_IS_SUPPLEMENTARY(c))
                    this->end+=2;
                else
                    this->end++;
            }
            this->textLength = this->end;
            extract_curr();
        }
        
        icu::CharacterIterator *clone() const
        {
            return new utf32_char_iter(*this);
        }

        
        virtual IteratorType base_iterator() const
        {
            return this->it;
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
            this->it = this->it_end;
            this->pos = this->end;
            this->index = 0;
            this->size = 0;
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
    };


    template<typename Iterator>
    basic_utf_char_iter<Iterator> *utf8_iterator(Iterator b,Iterator e)
    {
        return new utf8_char_iter<Iterator>(b,e);
    }
    
    template<typename Iterator>
    basic_utf_char_iter<Iterator> *mbcs_iterator(Iterator b,Iterator e,std::string const &encoding)
    {
        typedef typename std::iterator_traits<Iterator>::value_type char_type;

        // Free converter when needed

        struct lc_conv {
            UConverter *operator()() const
            {
                return ucv;
            }
            ~lc_conv()
            {
                if(ucv)
                    ucnv_close(ucv);
            }

            lc_conv(char const *enc)
            {
                UErrorCode err=U_ZERO_ERROR;
                ucv = ucnv_open(enc,&err);
                if(U_FAILURE(err)) {
                    if(ucv) ucnv_close(ucv);
                    check_and_throw_icu_error(err);
                }
            }
        private:
            UConverter *ucv;
        } cv(encoding.c_str());


        std::vector<char_type> buffer;

        size_t len = std::distance(b,e);
        buffer.resize(len+1);
        std::copy(b,e,buffer.begin());
        
        char_type const *begin = &buffer[0];
        char_type const *end = begin + len;

        std::vector<UChar> data;
        std::vector<size_t> offsets;

        data.reserve(len);
        offsets.reserve(len);

        char_type const *curr = begin;
        bool bmp_only = true;
        while(curr!=end) { 
            UErrorCode err=U_ZERO_ERROR;
            size_t offset = curr - begin;
            UChar32 c=ucnv_getNextUChar(cv(),&curr,end,&err);
            if(U_FAILURE(err))
                break;
            offsets.push_back(offset);
            if(U_IS_BMP(c)) {
                data.push_back(static_cast<UChar>(c));
            }
            else {
                offsets.push_back(offset);
                data.push_back(U16_LEAD(c));
                data.push_back(U16_TRAIL(c));
                bmp_only = false;
            }
        }
        
        offsets.push_back(len);
        if(bmp_only) {
            typedef mbcs_converter<Iterator,true> cvt_type;
            cvt_type cvt(b,data,offsets);
            return new mbcs_char_iter<Iterator,cvt_type>(cvt);
        }
        else {
            typedef mbcs_converter<Iterator,false> cvt_type;
            cvt_type cvt(b,data,offsets);
            return new mbcs_char_iter<Iterator,cvt_type>(cvt);
        }
    }
        
    
    template<typename Iterator>
    basic_utf_char_iter<Iterator> *sbcs_iterator(Iterator b,Iterator e,UChar const *table)
    {
        return new utf16_char_iter<Iterator,sbcs_converter>(b,e,sbcs_converter(table));
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


    template<typename CharType,int size=sizeof(CharType)>
    struct create_iterator;

    template<typename CharType>
    struct create_iterator<CharType,1> 
    {
        typedef CharType char_type;
        typedef basic_text_iterator<CharType> iterator_type;
        typedef basic_text<CharType> text_type;
        typedef basic_utf_char_iter<iterator_type> icu_iterator_type;

        static icu_iterator_type *get_iterator(text_type const &txt,std::string const &encoding)
        {
            if(is_utf8(encoding))
                return new utf8_char_iter<iterator_type>(txt.begin(),txt.end()); 
        }
        
    };
    basic_text_iterator<CharType> create_text_iterator(


} /// impl_icu
} //  locale
} // boost

#endif


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
