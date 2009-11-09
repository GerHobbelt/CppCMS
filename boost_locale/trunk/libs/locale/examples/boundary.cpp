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

int main(int argc,char **argv)
{
    using namespace boost::locale;
    using namespace std;

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
    
    cout.imbue(loc);
    // Set as default locale for output

    string text="Hello World! あにま! Linux2.6 and Windows7 is word and number. שָלוֹם עוֹלָם!";

    cout<<text<<endl;

    boundary::index_type index=boundary::map(boundary::word,text);

    for(unsigned i=0;i<index.size()-1;i++) {
        cout<<"Part ["<<string(text.begin()+index[i].offset,text.begin()+index[i+1].offset)<<"] has ";
        assert(index[i].next == index[i+1].prev);
        if(index[i].next & boundary::number)
            cout<<"number ";
        if(index[i].next & boundary::letter)
            cout<<"letter ";
        if(index[i].next & boundary::kana)
            cout<<"kana characters ";
        if(index[i].next & boundary::ideo)
            cout<<"ideographic characters ";
        cout<<endl;
    }

    index=boundary::map(boundary::character,text);
    
    for(unsigned i=0;i<index.size()-1;i++) {
        cout<<"|" << string(text.begin()+index[i].offset,text.begin()+index[i+1].offset) ;
    }
    cout<<"|\n\n";

    index=boundary::map(boundary::sentence,text);
    for(unsigned i=0;i<index.size()-1;i++) {
        cout<<"|" << string(text.begin()+index[i].offset,text.begin()+index[i+1].offset) ;
    }
    cout<<"|\n\n";
    index=boundary::map(boundary::line,text);
    for(unsigned i=0;i<index.size()-1;i++) {
        cout<<"|" << string(text.begin()+index[i].offset,text.begin()+index[i+1].offset) ;
    }
    cout<<"|\n\n";
    
    
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
