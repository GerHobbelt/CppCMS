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
#include <unicode/brkiter.h>


int main()
{

    std::string s="שלום עולם! ";
    for(int i=0;i<8;i++)
        s=s+s;
    icu::UnicodeString str(s.c_str(),"UTF-8");
    UChar const *ubegin = str.getTerminatedBuffer();
    UChar const *uend = ubegin+str.length();
#ifdef NATIVE
    icu::CharacterIterator *iter=new icu::StringCharacterIterator(str);
#else
    #ifdef USE_U16
        #if defined(BRIDGE)
            boost::locale::basic_text<UChar> txt(ubegin,uend);
            icu::CharacterIterator *iter=
                boost::locale::impl_icu::utf16_iterator<boost::locale::basic_text<UChar>::iterator>(txt.begin(),txt.end());
        #else
            icu::CharacterIterator *iter=boost::locale::impl_icu::utf16_iterator<UChar const *>(ubegin,uend);
        #endif
    #else
        std::list<char> l(s.begin(),s.end());
        #if defined(BRIDGE)
            #ifdef USE_LIST
            boost::locale::text txt(l.begin(),l.end());
            #else
            boost::locale::text txt(s.c_str(),s.c_str()+s.size());
            #endif
            icu::CharacterIterator *iter=boost::locale::impl_icu::utf8_iterator<boost::locale::text::iterator>(txt.begin(),txt.end());
        #else
            #ifdef USE_LIST
            icu::CharacterIterator *iter=boost::locale::impl_icu::utf8_iterator<std::list<char>::iterator>(l.begin(),l.end());
            #else
            icu::CharacterIterator *iter=boost::locale::impl_icu::utf8_iterator<char const *>(s.c_str(),s.c_str()+s.size());
            #endif
        #endif
    #endif
    iter->setToEnd();
    iter->setToStart();
    assert(iter->endIndex()!=0);
#endif

    UErrorCode e=U_ZERO_ERROR;
    icu::BreakIterator *biter = icu::BreakIterator::createWordInstance(icu::Locale::getUS(),e);
    biter->adoptText(iter);
    //biter->setText(utext_openUTF8(0,s.c_str(),s.size(),&e),e);
    //biter->setText(utext_openUChars(0,str.getTerminatedBuffer(),str.length(),&e),e);
    for(int i=0;i<10000;i++) {
        for(int indx=biter->first();indx!=icu::BreakIterator::DONE;indx=biter->next()) {
            //std::cout << indx << std::endl;
        }
    }

    delete biter;


}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
