//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include <booster/locale/collator.h>
#include <booster/locale/generator.h>
#include <booster/locale/localization_backend.h>
#include <iomanip>
#include "test_locale.h"


template<typename Char>
void test_comp(std::locale l,std::basic_string<Char> left,std::basic_string<Char> right,int ilevel,int expected)
{
    typedef std::basic_string<Char> string_type;
    booster::locale::collator_base::level_type level = static_cast<booster::locale::collator_base::level_type>(ilevel);
    TEST(booster::locale::comparator<Char>(l,level)(left,right) == (expected < 0));
    if(ilevel==4) {
        std::collate<Char> const &coll=std::use_facet<std::collate<Char> >(l);
        string_type lt=coll.transform(left.c_str(),left.c_str()+left.size());
        string_type rt=coll.transform(right.c_str(),right.c_str()+right.size());
        if(expected < 0)
            TEST(lt<rt);
        else if(expected == 0) {
            TEST(lt==rt);
        }
        else 
            TEST(lt > rt);
        long lh=coll.hash(left.c_str(),left.c_str()+left.size());
        long rh=coll.hash(right.c_str(),right.c_str()+right.size());
        if(expected == 0)
            TEST(lh==rh);
        else
            TEST(lh!=rh);
    }
    booster::locale::collator<Char> const &coll=std::use_facet<booster::locale::collator<Char> >(l);
    string_type lt=coll.transform(level,left.c_str(),left.c_str()+left.size());
    TEST(lt==coll.transform(level,left));
    string_type rt=coll.transform(level,right.c_str(),right.c_str()+right.size());
    TEST(rt==coll.transform(level,right));
    if(expected < 0)
        TEST(lt<rt);
    else if(expected == 0)
        TEST(lt==rt);
    else 
        TEST(lt > rt);
    long lh=coll.hash(level,left.c_str(),left.c_str()+left.size());
    TEST(lh==coll.hash(level,left));
    long rh=coll.hash(level,right.c_str(),right.c_str()+right.size());
    TEST(rh==coll.hash(level,right));
    if(expected == 0)
        TEST(lh==rh);
    else
        TEST(lh!=rh);

}    
        
#define TEST_COMP(c,_l,_r) test_comp<c>(l,_l,_r,level,expected)


void compare(std::string left,std::string right,int level,int expected)
{
    booster::locale::generator gen;
    std::locale l=gen("en_US.UTF-8");
    if(level == 4)
        TEST(l(left,right) == (expected < 0));
    TEST_COMP(char,left,right);
    TEST_COMP(wchar_t,to<wchar_t>(left),to<wchar_t>(right));
}


void test_collate()
{
    int
        primary     = 0,
        secondary   = 1,
        tertiary    = 2,
        quaternary  = 3,
        identical   = 4;
    int     le = -1,gt = 1,eq = 0;


    compare("a","A",primary,eq);
    compare("a","A",secondary,eq);
    compare("A","a",tertiary,gt);
    compare("a","A",tertiary,le);
    compare("a","A",quaternary,le);
    compare("A","a",quaternary,gt);
    compare("a","A",identical,le);
    compare("A","a",identical,gt);
    compare("a","??",primary,eq); //  a , ??
    compare("a","??",secondary,le); //  a , ??
    compare("??","a",secondary,gt); //  a , ??
    compare("a","??",quaternary,le); //  a , ??
    compare("??","a",quaternary,gt); //  a , ??
    compare("a","??",identical,le); //  a , ??
    compare("??","a",identical,gt); //  a , ??
}




int main()
{
    try {
        booster::locale::localization_backend_manager mgr = booster::locale::localization_backend_manager::global();
        mgr.select("winapi");
        booster::locale::localization_backend_manager::global(mgr);

        test_collate();
    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    FINALIZE();

}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
