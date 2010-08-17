//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/locale/formatting.hpp>
#include <boost/locale/localization_backend.hpp>
#include <boost/locale/generator.hpp>
#include <iomanip>
#include "test_locale.hpp"
#include "test_locale_tools.hpp"
#include <iostream>


template<typename CharType,typename RefCharType=CharType>
void test_by_char(std::locale const &l,std::locale const &lreal)
{
    typedef std::basic_stringstream<CharType> ss_type;
    typedef std::basic_stringstream<CharType> ss_ref_type;
    typedef std::basic_string<RefCharType> string_type;
    typedef std::basic_string<RefCharType> string_ref_type;

    using namespace boost::locale;

    {
        std::cout << "- Testing as::posix" << std::endl;
        ss_type ss;
        ss.imbue(l);

        ss << 1045.45;
        TEST(ss);
        double n;
        ss >> n;
        TEST(ss);
        TEST(n == 1045.45);
        TEST(ss.str()==to_correct_string<CharType>("1045.45",l));
    }

    {
        std::cout << "- Testing as::number" << std::endl;
        ss_type ss;
        ss.imbue(l);

        ss << as::number;
        ss << 1045.45;
        TEST(ss);
        double n;
        ss >> n;
        TEST(ss);
        TEST(n == 1045.45);

        ss_ref_type ss_ref;
        ss_ref.imbue(lreal);

        ss_ref << 1045.45;

        TEST(equal(ss.str(),ss_ref.str()));
    }

    {
        std::cout << "Testing as::currency" << std::endl;
        ss_type ss;
        ss.imbue(l);

        ss << as::currency;
        ss << 1043.34;
        ss << " ";
        ss << as::currency_iso ;
        ss << 1243.31;
        TEST(ss);
        double v1,v2;
        ss >> as::currency_national ;
        ss >> v1;
        TEST(ss);
        ss >> as::currency_iso ;
        ss >> v2 ;
        TEST(ss);
        TEST(v1==1043.34);
        TEST(v2==1243.31);

        ss_ref_type ss_ref;
        ss_ref.imbue(lreal);
        ss_ref << std::showbase;
        std::use_facet<std::money_put<RefCharType> >(lref).put(ss_ref,false,ss_ref,RefCharType(' '),104334);
        std::use_facet<std::money_put<RefCharType> >(lref).put(ss_ref,true,ss_ref,RefCharType(' '),124331);

        TEST(equal(ss.str(),ss_ref.str()));
    }

}

int main()
{
    try {
        boost::locale::localization_backend_manager mgr = boost::locale::localization_backend_manager::global();
        mgr.select("std");
        boost::locale::localization_backend_manager::global(mgr);
        boost::locale::generator gen;
    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    FINALIZE();

}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

