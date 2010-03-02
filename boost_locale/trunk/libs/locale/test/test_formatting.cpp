#include <boost/locale/formatting.hpp>
#include <boost/locale/generator.hpp>
#include "test_locale.hpp"
#include "test_locale_tools.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>


using namespace boost::locale;

//#define BOOST_NO_STD_WSTRING
//#define TESTEQ(x,y) do { std::cerr << x << " " << y << std::endl; TEST((x)==(y)); } while(0)
#define TESTEQ(x,y) TEST((x)==(y))

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
    //TEST_FP1(as::ordinal,1,"1st",int,1);
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
