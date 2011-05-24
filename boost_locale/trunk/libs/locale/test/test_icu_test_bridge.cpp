//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <iostream>
#include <libs/locale/src/icu/text_bridge.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <string.h>
#include <list>
#include <memory>

#include <assert.h>


#include <unicode/schriter.h>

#define TEST(x) assert(x)

void set_pos32(int pos,icu::CharacterIterator &ref,icu::CharacterIterator &it)
{
    ref.first32();
    it.first32();
    for(int i=0;i<pos;i++) {
        ref.next32();
        it.next32();
    }
}
void set_pos(int pos,icu::CharacterIterator &ref,icu::CharacterIterator &it)
{
    ref.first();
    it.first();
    for(int i=0;i<pos;i++) {
        ref.next();
        it.next();
    }
}

template<typename Iterator>
void test_iterator(icu::CharacterIterator &ref,icu::CharacterIterator &it,Iterator begin,int *distances)
{
    typedef boost::locale::impl_icu::basic_utf_char_iter<Iterator> actual_type;
    actual_type &actual = dynamic_cast<actual_type &>(it);
    struct eq {
        int *dist;
        Iterator b;
        actual_type &ac;
        icu::CharacterIterator &r,&i;
        void operator()()
        {
           if(dist[r.getIndex()] != std::distance(b,ac.base_iterator())) {
               std::cerr << r.getIndex() << " " << dist[r.getIndex()] << " " << std::distance(b,ac.base_iterator()) << std::endl;
           }
           TEST(dist[r.getIndex()] == std::distance(b,ac.base_iterator()));
        }
    } test_equal_base = { distances, begin, actual,ref,it };

    std::cout << "---- Forward Iteration first,next" << std::endl;
    
    {
        UChar cr,c;
        cr=ref.first();
        c=it.first();
        for(;;) {
            TEST(cr==c);
            TEST(ref.hasNext() == it.hasNext());
            TEST(ref.getIndex() == it.getIndex());
            TEST(ref.current() == it.current());
            test_equal_base();
            if(cr==icu::CharacterIterator::DONE) {
                break;
            }
            cr=ref.next();
            c=it.next();
        }
    }
    
    std::cout << "---- Forward Iteration first32,next32" << std::endl;
    
    {
        UChar32 cr,c;
        cr=ref.first32();
        c=it.first32();
        for(;;) {
            TEST(cr==c);
            TEST(ref.current32() == it.current32());
            TEST(ref.getIndex() == it.getIndex());
            test_equal_base();
            if(cr==icu::CharacterIterator::DONE) {
                break;
            }
            cr=ref.next32();
            c=it.next32();
        }
    }
    std::cout << "---- Forward Iteration postInc" << std::endl;
    
    {
        UChar cr,c;
        ref.first();
        it.first();
        for(;;) {
            TEST(ref.hasNext() == it.hasNext());
            cr = ref.nextPostInc();
            c = it.nextPostInc();
            TEST(cr==c);
            TEST(ref.getIndex() == it.getIndex());
            test_equal_base();
            if(cr==icu::CharacterIterator::DONE) {
                break;
            }
        }
    }
    std::cout << "---- Forward Iteration postInc32" << std::endl;
    
    {
        UChar32 cr,c;
        ref.first32();
        it.first32();
        for(;;) {
            TEST(ref.hasNext() == it.hasNext());
            cr = ref.next32PostInc();
            c = it.next32PostInc();
            TEST(cr==c);
            TEST(ref.getIndex() == it.getIndex());
            test_equal_base();
            if(cr==icu::CharacterIterator::DONE) {
                break;
            }
        }
    }

    std::cout << "---- Backward Iteration last/prev" << std::endl;
    
    {
        UChar cr,c;
        cr=ref.last();
        c=it.last();
        for(;;) {
            TEST(cr==c);
            TEST(ref.current() == it.current());
            TEST(ref.getIndex() == it.getIndex());
            test_equal_base();
            if(cr==icu::CharacterIterator::DONE) {
                break;
            }
            cr=ref.previous();
            c=it.previous();
        }
    }
    std::cout << "---- Backward Iteration last32/prev32" << std::endl;
    
    {
        UChar32 cr,c;
        cr=ref.last32();
        c=it.last32();
        for(;;) {
            if(cr!=c) {
                std::cerr << ref.getIndex() << " " << it.getIndex() << std::endl;
                std::cerr <<std::hex << cr << " " << c << std::endl;
            }
            TEST(cr==c);
            TEST(ref.current32() == it.current32());
            TEST(ref.getIndex() == it.getIndex());
            test_equal_base();
            if(cr==icu::CharacterIterator::DONE) {
                break;
            }
            cr=ref.previous32();
            c=it.previous32();
        }
    }
    
    {
        std::cout <<"---- Random Access setIndex" << std::endl;
        int len = ref.endIndex();
        for(int i=0;i<len+2;i++) {
            for(int j=0;j<len;j++){ 
                set_pos(j,ref,it);
                TEST(ref.setIndex(i) == it.setIndex(i));
                TEST(ref.current32() == it.current32());
                TEST(ref.getIndex() == it.getIndex());
                test_equal_base();
                TEST(ref.next() == it.next());
                TEST(ref.previous() == it.previous());
                TEST(ref.previous() == it.previous());
                test_equal_base();
            }
        }
    }
    
    {
        std::cout <<"---- Random Access setIndex32" << std::endl;
        int len = ref.endIndex();
        for(int i=0;i<len+2;i++) {
            for(int j=0;j<len;j++) {
                set_pos(j,ref,it);
                TEST(ref.setIndex(i) == it.setIndex(i));
                TEST(ref.current32() == it.current32());
                TEST(ref.getIndex() == it.getIndex());
                test_equal_base();
                TEST(ref.next() == it.next());
                TEST(ref.previous() == it.previous());
                TEST(ref.previous() == it.previous());
                test_equal_base();
            }
        }
    }
    
    {
        std::cout <<"---- Random Access move" << std::endl;
        int len = ref.endIndex();
        for(int i=-len+2;i<len+2;i++) {
            for(int j=0;j<len;j++) {
                set_pos(j,ref,it);
                TEST(ref.move(i,icu::CharacterIterator::kStart) == it.move(i,icu::CharacterIterator::kStart));
                TEST(ref.current32() == it.current32());
                TEST(ref.getIndex() == it.getIndex());
                test_equal_base();
                TEST(ref.next() == it.next());
                TEST(ref.previous() == it.previous());
                TEST(ref.previous() == it.previous());
                test_equal_base();
            }
            for(int j=0;j<len;j++) {
                set_pos(j,ref,it);
                TEST(ref.move(i,icu::CharacterIterator::kCurrent) == it.move(i,icu::CharacterIterator::kCurrent));
                TEST(ref.current32() == it.current32());
                TEST(ref.getIndex() == it.getIndex());
                test_equal_base();
                TEST(ref.next() == it.next());
                TEST(ref.previous() == it.previous());
                TEST(ref.previous() == it.previous());
                test_equal_base();
            }
            for(int j=0;j<len;j++) {
                set_pos(j,ref,it);
                TEST(ref.move(i,icu::CharacterIterator::kEnd) == it.move(i,icu::CharacterIterator::kEnd));
                TEST(ref.current32() == it.current32());
                TEST(ref.getIndex() == it.getIndex());
                test_equal_base();
                TEST(ref.next() == it.next());
                TEST(ref.previous() == it.previous());
                TEST(ref.previous() == it.previous());
                test_equal_base();
            }
        }
    }

    {
        std::cout <<"---- Random Access move32" << std::endl;
        int len = ref.endIndex();
        for(int i=-len+2;i<len+2;i++) {
            for(int j=0;j<len;j++) {
                set_pos32(j,ref,it);
                TEST(ref.move32(i,icu::CharacterIterator::kStart) == it.move32(i,icu::CharacterIterator::kStart));
                TEST(ref.current32() == it.current32());
                TEST(ref.getIndex() == it.getIndex());
                TEST(ref.next() == it.next());
                TEST(ref.previous() == it.previous());
                TEST(ref.previous() == it.previous());
                test_equal_base();
            }
            for(int j=0;j<len;j++) {
                set_pos32(j,ref,it);
                TEST(ref.move32(i,icu::CharacterIterator::kCurrent) == it.move32(i,icu::CharacterIterator::kCurrent));
                TEST(ref.current32() == it.current32());
                TEST(ref.getIndex() == it.getIndex());
                TEST(ref.next() == it.next());
                TEST(ref.previous() == it.previous());
                TEST(ref.previous() == it.previous());
                test_equal_base();
            }
            for(int j=0;j<len;j++) {
                set_pos32(j,ref,it);
                TEST(ref.move32(i,icu::CharacterIterator::kEnd) == it.move32(i,icu::CharacterIterator::kEnd));
                TEST(ref.current32() == it.current32());
                TEST(ref.getIndex() == it.getIndex());
                TEST(ref.next() == it.next());
                TEST(ref.previous() == it.previous());
                TEST(ref.previous() == it.previous());
                test_equal_base();
            }
        }
    }


    
}

template<typename CharType,bool bmp=false,int size=sizeof(CharType)>
struct create_iterator_traits;

template<typename CharType,int size>
struct create_iterator_traits<CharType,true,size>
{
    template<typename IteratorType>
    static icu::CharacterIterator *create(IteratorType b,IteratorType e)
    {
        return boost::locale::impl_icu::latin1_iterator<IteratorType>(b,e);
    }
};

template<typename CharType>
struct create_iterator_traits<CharType,false,1>
{
    template<typename IteratorType>
    static icu::CharacterIterator *create(IteratorType b,IteratorType e)
    {
        return boost::locale::impl_icu::utf8_iterator<IteratorType>(b,e);
    }
};

template<typename CharType>
struct create_iterator_traits<CharType,false,2>
{
    template<typename IteratorType>
    static icu::CharacterIterator *create(IteratorType b,IteratorType e)
    {
        return boost::locale::impl_icu::utf16_iterator<IteratorType>(b,e);
    }
};

template<typename CharType>
struct create_iterator_traits<CharType,false,4>
{
    template<typename IteratorType>
    static icu::CharacterIterator *create(IteratorType b,IteratorType e)
    {
        return boost::locale::impl_icu::utf32_iterator<IteratorType>(b,e);
    }
};



template<typename IteratorType,bool bmp>
void test_single_iter(IteratorType begin,IteratorType end,int *distances,icu::UnicodeString &ustr)
{
    typedef typename std::iterator_traits<IteratorType>::value_type char_type;
    std::cout << "--- Normal Iterator" << std::endl;
    {
        std::auto_ptr<icu::CharacterIterator> ref(new icu::StringCharacterIterator(ustr));
        std::auto_ptr<icu::CharacterIterator> it(create_iterator_traits<char_type,bmp>::create(begin,end));
        test_iterator(*ref,*it,begin,distances);
    }
    std::cout << "--- Boost.Locale text iterator" << std::endl;
    {
        boost::locale::basic_text<char_type> txt(begin,end);
        typedef typename boost::locale::basic_text<char_type>::iterator iterator;
        std::auto_ptr<icu::CharacterIterator> ref(new icu::StringCharacterIterator(ustr));
        std::auto_ptr<icu::CharacterIterator> it(create_iterator_traits<char_type,bmp>::create(txt.begin(),txt.end()));
        test_iterator(*ref,*it,txt.begin(),distances);
    }
};

template<typename CharType,bool bmp>
void test_all(icu::UnicodeString &ustr,CharType const *str,int *distrances)
{

    CharType const *sbegin = str;
    CharType const *send = str;
    while(*send)
        send++;

    {
        
        std::cout << "-- char_type const *" << std::endl;
        test_single_iter<CharType const *,bmp>(sbegin,send,distrances,ustr);
    }

    {
        std::cout << "-- std::basic_string<>::iterator" << std::endl;
        std::basic_string<CharType> s(str);
        test_single_iter<typename std::basic_string<CharType>::iterator,bmp>(s.begin(),s.end(),distrances,ustr);
    }

    {
        std::cout << "-- std::list<char>::iterator" << std::endl;
        std::list<CharType> s(sbegin,send);
        test_single_iter<typename std::list<CharType>::iterator,bmp>(s.begin(),s.end(),distrances,ustr);
    }
}

void create_index_utf8(char const * const *str,int *dst,char *out)
{
    for(;;) {
        char const *cur = *str++;
        *dst++ = strlen(out);
        if(cur == 0)
            break;
        if(strlen(cur) == 4) { // utf-16 index
            *dst++ = strlen(out);
        }
        strcat(out,cur);
    }
    *dst++ = 0;
}

void create_index_utf16(char const * const *str,int *dst)
{
    int dist = 0;
    for(;;) {
        char const *cur = *str++;
        *dst++ = dist;
        if(cur == 0)
            break;
        if(strlen(cur) == 4) { // utf-16 index
            *dst++ = dist;
            dist+=2;
        }
        else {
            dist++;
        }
    }
    *dst++ = 0;
}

void create_index_utf32(char const * const *str,int *dst)
{
    int dist = 0;
    for(;;) {
        char const *cur = *str++;
        *dst++ = dist;
        if(cur == 0)
            break;
        if(strlen(cur) == 4) { // utf-16 index
            *dst++ = dist;
        }
        dist++;
    }
    *dst++ = 0;
}



void run_test_on_sample(char const * const *str_set)
{
    int dist[256] = {0};
    char str[256] = {0};
    
    std::cout << "- UTF-8" << std::endl;

    create_index_utf8(str_set,dist,str);
    
    for(size_t i=0;dist[i+1]!=0;i++) {
        std::cout<<"|";
        std::cout.write(str+dist[i],dist[i+1]-dist[i]);
    }
    std::cout << "|" << std::endl;
    std::cout << str << std::endl;


    icu::UnicodeString ustr(str,"UTF-8");

    test_all<char,false>(ustr,str,dist);

    std::cout << "- UTF-16" << std::endl;
    {
        create_index_utf16(str_set,dist);
        test_all<UChar,false>(ustr,ustr.getTerminatedBuffer(),dist);
    }

    std::cout << "- UTF-32" << std::endl;
    {
        int n=ustr.countChar32();
        std::basic_string<UChar32> u32;
        for(int i=0;i<n;i++) {
            u32+=ustr.char32At(ustr.moveIndex32(0,i));
        }
        create_index_utf32(str_set,dist);
        test_all<UChar32,false>(ustr,u32.c_str(),dist);
    }
}

template<typename IteratorType>
void test_single_mbcs(IteratorType begin,IteratorType end,int *distances,icu::UnicodeString &ustr)
{
    typedef typename std::iterator_traits<IteratorType>::value_type char_type;
    std::cout << "--- Normal Iterator" << std::endl;
    {
        std::auto_ptr<icu::CharacterIterator> ref(new icu::StringCharacterIterator(ustr));
        std::auto_ptr<icu::CharacterIterator> it(boost::locale::impl_icu::mbcs_iterator<IteratorType>(begin,end,"UTF-8"));
        test_iterator(*ref,*it,begin,distances);
    }
    std::cout << "--- Boost.Locale text iterator" << std::endl;
    {
        boost::locale::text txt(begin,end);
        typedef boost::locale::text::iterator iterator;
        std::auto_ptr<icu::CharacterIterator> ref(new icu::StringCharacterIterator(ustr));
        std::auto_ptr<icu::CharacterIterator> it(boost::locale::impl_icu::mbcs_iterator<iterator>(txt.begin(),txt.end(),"UTF-8"));
        test_iterator(*ref,*it,txt.begin(),distances);
    }
}

void test_all_mbcs(icu::UnicodeString &ustr,char const *str,int *distrances)
{

    char const *sbegin = str;
    char const *send = str;
    while(*send)
        send++;

    {
        
        std::cout << "-- char_type const *" << std::endl;
        test_single_mbcs<char const *>(sbegin,send,distrances,ustr);
    }

    {
        std::cout << "-- std::basic_string<>::iterator" << std::endl;
        std::string s(str);
        test_single_mbcs<std::string::iterator>(s.begin(),s.end(),distrances,ustr);
    }

    {
        std::cout << "-- std::list<char>::iterator" << std::endl;
        std::list<char> s(sbegin,send);
        test_single_mbcs<std::list<char>::iterator>(s.begin(),s.end(),distrances,ustr);
    }
}



void run_test_on_mbcs(char const * const *str_set)
{
    int dist[256] = {0};
    char str[256] = {0};
    
    std::cout << "- MBCS (full conversion)" << std::endl;

    create_index_utf8(str_set,dist,str);

    icu::UnicodeString ustr(str,"UTF-8");

    test_all_mbcs(ustr,str,dist);

}


template<typename CharType,bool bmp>
void test_all_mbcs(icu::UnicodeString &ustr,CharType const *str,int *distrances)
{

    CharType const *sbegin = str;
    CharType const *send = str;
    while(*send)
        send++;

    {
        
        std::cout << "-- char_type const *" << std::endl;
        test_single_iter<CharType const *,bmp>(sbegin,send,distrances,ustr);
    }

    {
        std::cout << "-- std::basic_string<>::iterator" << std::endl;
        std::basic_string<CharType> s(str);
        test_single_iter<typename std::basic_string<CharType>::iterator,bmp>(s.begin(),s.end(),distrances,ustr);
    }

    {
        std::cout << "-- std::list<char>::iterator" << std::endl;
        std::list<CharType> s(sbegin,send);
        test_single_iter<typename std::list<CharType>::iterator,bmp>(s.begin(),s.end(),distrances,ustr);
    }
}





void run_test_on_bmp_sample(char const *str)
{

    icu::UnicodeString ustr(str,"ISO-8859-1");

    int dist[256];
    for(int i=0;i<256;i++)
        dist[i]=i;
    
    std::cout << "- Latin 1 for char *" << std::endl;
    test_all<char,true>(ustr,str,dist);

    std::cout << "- UCS-2 for UChar *" << std::endl;
    test_all<UChar,true>(ustr,ustr.getTerminatedBuffer(),dist);
}





int main()
{
    {
        char const * const str[]={"ש","ל","ו","ם"," ","ע","ו","ל","ם"," ","𐌳","𐍉","א"," ","日","本","語","!",0};
        std::cout << "Sample 1: mixed 1,2,3,4 utf8 lengthes" << std::endl;
        run_test_on_sample(str);
        run_test_on_mbcs(str);
    }
    {
        char const * const str[]={"𐌳","א"," ","日","本","語","!",0};
        std::cout << "Sample 2: Start with non-BMP" << std::endl;
        run_test_on_sample(str);
        run_test_on_mbcs(str);
    }
    {
        char const * const str[]={"ש","ל","𐍉",0};
        std::cout << "Sample 3: end with non-BMP" << std::endl;
        run_test_on_sample(str);
        run_test_on_mbcs(str);
    }
    {
        char const * const str[]={"ש","ל","ו","ם"," ","ע","ו","ל","ם",0};
        std::cout << "Sample 4: with BMP only" << std::endl;
        run_test_on_sample(str);
        run_test_on_mbcs(str);
    }
    {
        char const * const str[]={0};
        std::cout << "Sample 5: Empty" << std::endl;
        run_test_on_sample(str);
        run_test_on_mbcs(str);
    }
    {
        std::cout << "Sample 6: BMP/UTF-16 text" << std::endl;
        run_test_on_bmp_sample("Hello World \xD0\xD1 \xA9 ");
    }
    {
        std::cout << "Sample 7: BMP/UTF-16 Empty text" << std::endl;
        run_test_on_bmp_sample("");
    }

}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
