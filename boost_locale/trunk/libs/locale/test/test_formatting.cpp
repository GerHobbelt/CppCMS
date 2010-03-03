#include <boost/locale/formatting.hpp>
#include <boost/locale/generator.hpp>
#include "test_locale.hpp"
#include "test_locale_tools.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>

#include <unicode/uversion.h>

using namespace boost::locale;

//#define TEST_DEBUG

#ifdef TEST_DEBUG
#define BOOST_NO_STD_WSTRING
#undef BOOST_HAS_CHAR16_T
#undef BOOST_HAS_CHAR32_T
#define TESTEQ(x,y) do { std::cerr << x << " " << y << std::endl; TEST((x)==(y)); } while(0)
#else
#define TESTEQ(x,y) TEST((x)==(y))
#endif

#define TEST_FMT(manip,value,expected) \
do{ \
    std::basic_ostringstream<CharType> ss; \
    ss.imbue(loc); \
    ss << manip << value; \
    TESTEQ(ss.str(),to_correct_string<CharType>(expected,loc)); \
}while(0)

#define TEST_PAR(manip,type,actual,expected) \
do{ \
    type v; \
    {std::basic_istringstream<CharType> ss; \
    ss.imbue(loc); \
    ss.str(to_correct_string<CharType>(actual,loc)); \
    ss >> manip >> v >> std::ws; \
    TESTEQ(v,expected); \
    TEST(ss.eof()); }\
    {std::basic_istringstream<CharType> ss; \
    ss.imbue(loc); \
    ss.str(to_correct_string<CharType>(std::string(actual)+"@",loc)); \
    CharType tmp_c; \
    ss >> manip >> v >> std::skipws >> tmp_c; \
    TESTEQ(v,expected); \
    TEST(tmp_c=='@'); } \
}while(0)

#define TEST_FP1(manip,value_in,str,type,value_out) \
do { \
    TEST_FMT(manip,value_in,str); \
    TEST_PAR(manip,type,str,value_out); \
}while(0)

#define TEST_FP2(m1,m2,value_in,str,type,value_out) \
do { \
    TEST_FMT(m1<<m2,value_in,str); \
    TEST_PAR(m1>>m2,type,str,value_out);  \
}while(0)

#define TEST_FP3(m1,m2,m3,value_in,str,type,value_out) \
do { \
    TEST_FMT(m1<<m2<<m3,value_in,str); \
    TEST_PAR(m1>>m2>>m3,type,str,value_out); \
}while(0)

#define TEST_FP4(m1,m2,m3,m4,value_in,str,type,value_out) \
do { \
    TEST_FMT(m1<<m2<<m3<<m4,value_in,str); \
    TEST_PAR(m1>>m2>>m3>>m4,type,str,value_out); \
}while(0)



template<typename CharType>
void test_manip()
{
    boost::locale::generator g;
    std::locale loc=g("en_US.UTF-8");
    
    TEST_FP1(as::posix,1200.1,"1200.1",double,1200.1);
    TEST_FP1(as::number,1200.1,"1,200.1",double,1200.1);
    TEST_FMT(as::number<<std::setfill(CharType('_'))<<std::setw(6),1534,"_1,534");
    TEST_FMT(as::number<<std::left<<std::setfill(CharType('_'))<<std::setw(6),1534,"1,534_");
    TEST_FP3(as::number,std::left,std::setw(3),15,"15 ",int,15);
    TEST_FP3(as::number,std::right,std::setw(3),15," 15",int,15);
    TEST_FP3(as::number,std::setprecision(3),std::fixed,13.1,"13.100",double,13.1);
    TEST_FP3(as::number,std::setprecision(3),std::scientific,13.1,"1.310E1",double,13.1);
    TEST_FP1(as::percent,0.1,"10%",double,0.1);
    TEST_FP3(as::percent,std::fixed,std::setprecision(1),0.10,"10.0%",double,0.1);
    TEST_FP1(as::currency,1345,"$1,345.00",int,1345);
    TEST_FP1(as::currency,1345.34,"$1,345.34",double,1345.34);
    #if U_ICU_VERSION_MAJOR_NUM*100 + U_ICU_VERSION_MINOR_NUM >= 402
    TEST_FP2(as::currency,as::currency_national,1345,"USD1,345.00",int,1345);
    TEST_FP2(as::currency,as::currency_national,1345.34,"USD1,345.34",double,1345.34);
    #endif
    TEST_FP1(as::spellout,132,"one hundred and thirty-two",int,132);
    loc=g("he_IL.UTF-8");
    TEST_FP1(as::ordinal,3,"שלישי",int,3);
    loc=g("en_US.UTF-8");

    time_t a_date = 3600*24*(31+4); // Feb 5th
    time_t a_time = 3600*15+60*33; // 15:33:05
    time_t a_timesec = 13;
    time_t a_datetime = a_date + a_time + a_timesec;

    TEST_FP2(as::date,                as::gmt,a_datetime,"Feb 5, 1970",time_t,a_date);
    TEST_FP3(as::date,as::date_short ,as::gmt,a_datetime,"2/5/70",time_t,a_date);
    TEST_FP3(as::date,as::date_medium,as::gmt,a_datetime,"Feb 5, 1970",time_t,a_date);
    TEST_FP3(as::date,as::date_long  ,as::gmt,a_datetime,"February 5, 1970",time_t,a_date);
    TEST_FP3(as::date,as::date_full  ,as::gmt,a_datetime,"Thursday, February 5, 1970",time_t,a_date);
    
    TEST_FP2(as::time,                as::gmt,a_datetime,"3:33:13 PM",time_t,a_time+a_timesec);
    TEST_FP3(as::time,as::time_short ,as::gmt,a_datetime,"3:33 PM",time_t,a_time);
    TEST_FP3(as::time,as::time_medium,as::gmt,a_datetime,"3:33:13 PM",time_t,a_time+a_timesec);
    TEST_FP3(as::time,as::time_long  ,as::gmt,a_datetime,"3:33:13 PM GMT+00:00",time_t,a_time+a_timesec);
    TEST_FP3(as::time,as::time_full  ,as::gmt,a_datetime,"3:33:13 PM GMT+00:00",time_t,a_time+a_timesec);

    TEST_FP2(as::time,                as::time_zone("GMT+01:00"),a_datetime,"4:33:13 PM",time_t,a_time+a_timesec);
    TEST_FP3(as::time,as::time_short ,as::time_zone("GMT+01:00"),a_datetime,"4:33 PM",time_t,a_time);
    TEST_FP3(as::time,as::time_medium,as::time_zone("GMT+01:00"),a_datetime,"4:33:13 PM",time_t,a_time+a_timesec);
    TEST_FP3(as::time,as::time_long  ,as::time_zone("GMT+01:00"),a_datetime,"4:33:13 PM GMT+01:00",time_t,a_time+a_timesec);
    TEST_FP3(as::time,as::time_full  ,as::time_zone("GMT+01:00"),a_datetime,"4:33:13 PM GMT+01:00",time_t,a_time+a_timesec);

    TEST_FP2(as::datetime,                                as::gmt,a_datetime,"Feb 5, 1970 3:33:13 PM",time_t,a_datetime);
    TEST_FP4(as::datetime,as::date_short ,as::time_short ,as::gmt,a_datetime,"2/5/70 3:33 PM",time_t,a_date+a_time);
    TEST_FP4(as::datetime,as::date_medium,as::time_medium,as::gmt,a_datetime,"Feb 5, 1970 3:33:13 PM",time_t,a_datetime);
    TEST_FP4(as::datetime,as::date_long  ,as::time_long  ,as::gmt,a_datetime,"February 5, 1970 3:33:13 PM GMT+00:00",time_t,a_datetime);
    TEST_FP4(as::datetime,as::date_full  ,as::time_full  ,as::gmt,a_datetime,"Thursday, February 5, 1970 3:33:13 PM GMT+00:00",time_t,a_datetime);

    time_t now=time(0);
    char local_time_str[256];
    std::string format="%H:%M:%S";
    std::basic_string<CharType> format_string(format.begin(),format.end());
    strftime(local_time_str,sizeof(local_time_str),format.c_str(),localtime(&now));
    TEST_FMT(as::ftime(format_string),now,local_time_str);
    TEST_FMT(as::ftime(format_string)<<as::gmt<<as::local_time,now,local_time_str);

    std::string marks =  
        "aAbB" 
        "cdeh"
        "HIjm"
        "Mnpr"
        "RStT"
        "xXyY"
        "Z%";

    std::string result[]= { 
        "Thu","Thursday","Feb","February",  // aAbB
        "Thursday, February 5, 1970 3:33:13 PM GMT+00:00","05","5","Feb", // cdeh
        "15","03","36","02", // HIjm
        "33","\n","PM", "03:33:13 PM",// Mnpr
        "15:33","13","\t","15:33:13", // RStT
        "Feb 5, 1970","3:33:13 PM","70","1970", // xXyY
        "GMT+00:00","%" }; /// Z%

    for(unsigned i=0;i<marks.size();i++) {
        format_string.clear();
        format_string+=static_cast<CharType>('%');
        format_string+=static_cast<CharType>(marks[i]);
        TEST_FMT(as::ftime(format_string)<<as::gmt,a_datetime,result[i]);
    }

    /*std::string sample="Now is %A, %H o'clock ' or not ' ";
    format_string.assign(sample.begin(),sample.end());
    TEST_FMT(as::ftime(format_string)<<as::gmt,a_datetime,"Now is Thursday, 15 o'clock ' or not ' ");*/

}


int main()
{
    try {
        std::cout << "Testing char" << std::endl;
        test_manip<char>();

        #ifndef BOOST_NO_STD_WSTRING
        std::cout << "Testing wchar_t" << std::endl;
        test_manip<wchar_t>();
        #endif

        #ifdef BOOST_HAS_CHAR16_T
        std::cout << "Testing char16_t" << std::endl;
        test_manip<char16_t>();
        #endif

        #ifdef BOOST_HAS_CHAR32_T
        std::cout << "Testing char32_t" << std::endl;
        test_manip<char32_t>();
        #endif

    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Ok" << std::endl;
    return EXIT_SUCCESS;

}


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
