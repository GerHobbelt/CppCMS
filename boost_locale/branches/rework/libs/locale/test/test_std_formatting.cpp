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
#include <boost/locale/codepage.hpp>
#include <iomanip>
#include "test_locale.hpp"
#include "test_locale_tools.hpp"
#include <iostream>


template<typename C1,typename C2>
bool equal(std::basic_string<C1> const &s1,std::basic_string<C2> const &s2)
{
    return boost::locale::conv::from_utf(s1,"UTF-8") == boost::locale::conv::from_utf(s2,"UTF-8");
}

template<>
bool equal(std::string const &s1,std::string const &s2)
{
    return s1==s2;
}

template<typename CharType>
std::basic_string<CharType> conv_to_char(char const *p)
{
    std::basic_string<CharType> r;
    while(*p)
        r+=CharType(*p++);
    return r;
}


template<typename CharType,typename RefCharType>
void test_by_char(std::locale const &l,std::locale const &lreal)
{
    typedef std::basic_stringstream<CharType> ss_type;
    typedef std::basic_stringstream<RefCharType> ss_ref_type;
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
        std::cout << "- Testing as::currency" << std::endl;
        ss_type ss;
        ss.imbue(l);

        ss << as::currency;
        ss << 1043.34;
        ss << CharType(' ');
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
        std::use_facet<std::money_put<RefCharType> >(lreal).put(ss_ref,false,ss_ref,RefCharType(' '),104334);
        ss_ref << RefCharType(' ');
        std::use_facet<std::money_put<RefCharType> >(lreal).put(ss_ref,true,ss_ref,RefCharType(' '),124331);

        TEST(equal(ss.str(),ss_ref.str()));
    }
    {
        std::cout << "- Testing as::date/time" << std::endl;
        ss_type ss;
        ss.imbue(l);
        
        time_t a_date = 3600*24*(31+4); // Feb 5th
        time_t a_time = 3600*15+60*33; // 15:33:05
        time_t a_timesec = 13;
        time_t a_datetime = a_date + a_time + a_timesec;

        ss << as::time_zone("GMT");

        ss << as::date << a_datetime << CharType('\n');
        ss << as::time << a_datetime << CharType('\n');
        ss << as::datetime << a_datetime << CharType('\n');
        ss << as::time_zone("GMT+01:00");
        ss << as::ftime(conv_to_char<CharType>("%H")) << a_datetime << CharType('\n');
        ss << as::time_zone("GMT+00:15");
        ss << as::ftime(conv_to_char<CharType>("%M")) << a_datetime << CharType('\n');

        ss_ref_type ss_ref;
        ss_ref.imbue(lreal);

        std::basic_string<RefCharType> rfmt(conv_to_char<RefCharType>("%x\n%X\n%c\n16\n48\n"));

        std::tm tm=*gmtime(&a_datetime);

        std::use_facet<std::time_put<RefCharType> >(lreal).put(ss_ref,ss_ref,RefCharType(' '),&tm,rfmt.c_str(),rfmt.c_str()+rfmt.size());

        TEST(equal(ss.str(),ss_ref.str()));
        //std::cerr << "[" << boost::locale::conv::from_utf(ss.str(),"UTF-8") << "]=\n" ;
        //std::cerr << "[" << boost::locale::conv::from_utf(ss_ref.str(),"UTF-8") << "]\n" ;
    }

}

int main()
{
    try {
        boost::locale::localization_backend_manager mgr = boost::locale::localization_backend_manager::global();
        mgr.select("std");
        boost::locale::localization_backend_manager::global(mgr);
        boost::locale::generator gen;

        {
            std::cout << "UTF locale" << std::endl;
            std::string real_name;
            std::string name = get_std_name("en_US.UTF-8",&real_name);
            if(name.empty()) {
                std::cout << "en_US.UTF-8 not supported" << std::endl;
            }
            else {
                std::locale l1=gen(name),l2(real_name.c_str());
                std::cout << "UTF-8" << std::endl;
                if(name==real_name) 
                    test_by_char<char,char>(l1,l2);
                else
                    #ifndef BOOST_NO_STD_WSTRING
                    test_by_char<char,wchar_t>(l1,l2);
                    #else
                    std::cout << "UTF-8 Unsupported - no wide locale" << std::endl;
                    #endif
                
                #ifndef BOOST_NO_STD_WSTRING
                std::cout << "Wide UTF-" << sizeof(wchar_t) * 8 << std::endl;
                test_by_char<wchar_t,wchar_t>(l1,l2);
                #endif

                #ifdef BOOST_HAS_CHAR16_T
                std::cout << "char16 UTF-16" << std::endl;
                test_by_char<char16_t,char16_t>(l1,l2);
                #endif
                #ifdef BOOST_HAS_CHAR32_T
                std::cout << "char32 UTF-32" << std::endl;
                test_by_char<char32_t,char32_t>(l1,l2);
                #endif
            }
        }
        {
            std::cout << "Latin-1 locale" << std::endl;
            std::string real_name;
            std::string name = get_std_name("en_US.ISO-8859-1",&real_name);
            if(name.empty()) {
                std::cout << "en_US.ISO-8859-8 not supported" << std::endl;
            }
            else {
                std::locale l1=gen(name),l2(real_name.c_str());
                test_by_char<char,char>(l1,l2);
                #ifndef BOOST_NO_STD_WSTRING
                std::cout << "Wide UTF-" << sizeof(wchar_t) * 8 << std::endl;
                test_by_char<wchar_t,wchar_t>(l1,l2);
                #endif

                #ifdef BOOST_HAS_CHAR16_T
                std::cout << "char16 UTF-16" << std::endl;
                test_by_char<char16_t,char16_t>(l1,l2);
                #endif
                #ifdef BOOST_HAS_CHAR32_T
                std::cout << "char32 UTF-32" << std::endl;
                test_by_char<char32_t,char32_t>(l1,l2);
                #endif
            }
        }
        {
            std::cout << "Testing UTF-8 punct workaround" << std::endl;
            std::string real_name;
            std::string name = get_std_name("ru_RU.UTF-8",&real_name);
            if(name.empty()) {
                std::cout << "- No russian locale" << std::endl;
            }
            else {
                std::locale l1=gen(name),l2(real_name.c_str());
                bool fails = false;
                try {
                    std::ostringstream ss;
                    ss.imbue(l2);
                    ss << 12345.45;
                    boost::locale::conv::from_utf<char>(ss.str(),"windows-1251",boost::locale::conv::stop);
                    fails = false;
                }
                catch(...) {
                    fails = true;
                }
                
                if(!fails) {
                    std::cout << "- No invalid UTF. No need to check"<<std::endl;
                }
                else {
                    std::ostringstream ss;
                    ss.imbue(l1);
                    ss << std::setprecision(10) ;
                    ss << boost::locale::as::number << 12345.45;
                    #ifdef BOOST_NO_STD_WSTRING
                    TEST(ss.str() == "12345,45");
                    #else
                    TEST(ss.str() == "12 345,45");
                    #endif
                }
            }
        }
    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    FINALIZE();

}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

