//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/locale.hpp>
#include <iostream>
#include <cassert>
#include <ctime>

#ifndef BOOST_NO_SWPRINTF    
int main(int argc,char **argv)
{
    using namespace boost::locale;
    using namespace std;

    std::ios_base::sync_with_stdio(false);

    generator gen;
    locale::global(locale(""));
    locale loc;
    if(argc == 1)
        loc=gen(""); 
    else if(argc == 2)
        loc=gen(argv[1]);
    else
        loc=gen(argv[1],argv[2]);

    // Create system default locale

    locale::global(loc); 
    // Make it system global
    
    wcout.imbue(loc);
    // Set as default locale for output

    wstring text=L"Hello World! あにま! Linux2.6 and Windows7 is word and number. שָלוֹם עוֹלָם!";

    wcout<<text<<endl;

    typedef boundary::token_iterator<std::wstring::iterator> iter_type;
    typedef boundary::mapping<iter_type>  mapping_type;
    mapping_type index(boundary::word,text.begin(),text.end());
    iter_type p,e;

    for(p=index.begin(),e=index.end();p!=e;++p) {
        wcout<<L"Part ["<<*p<<L"] has ";
        if(p.mark() & boundary::word_number)
            wcout<<L"number ";
        if(p.mark() & boundary::word_letter)
            wcout<<L"letter ";
        if(p.mark() & boundary::word_kana)
            wcout<<L"kana characters ";
        if(p.mark() & boundary::word_ideo)
            wcout<<L"ideographic characters ";
        wcout<<endl;
    }

    index.map(boundary::character,text.begin(),text.end());

    for(p=index.begin(),e=index.end();p!=e;++p) {
        wcout<<L"|" <<*p ;
    }
    wcout<<L"|\n\n";

    index.map(boundary::line,text.begin(),text.end());

    for(p=index.begin(),e=index.end();p!=e;++p) {
        wcout<<L"|" <<*p ;
    }
    wcout<<L"|\n\n";

    index.map(boundary::sentence,text.begin(),text.end());

    for(p=index.begin(),e=index.end();p!=e;++p) {
        wcout<<L"|" <<*p ;
    }
    wcout<<"|\n\n";
    
}
#else
int main()
{
    std::cout<<"This platform does not support wcout"<<std::endl;
}
#endif


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
