//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/locale.hpp>
#include <iostream>
#include <cassert>
#include <ctime>

int main()
{
    using namespace boost::locale;
    using namespace std;

    generator gen;
    // Make system default locale global
    std::locale loc = gen("");
    locale::global(loc); 
    cout.imbue(loc);
    

    //string text="Hello World! あにま! Linux2.6 and Windows7 is word and number. שָלוֹם עוֹלָם!";
    string text="Hello!How\nAre you?\n";

    cout<<text<<endl;

    boundary::stoken_index index(boundary::sentence,text.begin(),text.end());
    index.rule(boundary::sentence_term);
    //index.full_select(true);
    boundary::stoken_index::iterator p,e;

    for(p=index.begin(),e=index.end();p!=e;++p) {
        cout <<"["<<*p<<"]" << " "  << std::hex << p->rule()<< endl;
    }
    return 0;

    index.map(boundary::character,text.begin(),text.end());

    for(p=index.begin(),e=index.end();p!=e;++p) {
        cout<<"|" <<*p ;
    }
    cout<<"|\n\n";

    index.map(boundary::line,text.begin(),text.end());

    for(p=index.begin(),e=index.end();p!=e;++p) {
        cout<<"|" <<*p ;
    }
    cout<<"|\n\n";

    index.map(boundary::sentence,text.begin(),text.end());

    for(p=index.begin(),e=index.end();p!=e;++p) {
        cout<<"|" <<*p ;
    }
    cout<<"|\n\n";
    
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
