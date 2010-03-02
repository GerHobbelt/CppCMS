#include <boost/locale/formatting.hpp>
#include <boost/locale/generator.hpp>
#include "test_locale.hpp"
#include "test_locale_tools.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>


using namespace boost::locale;


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
    std::basic_istringstream<CharType> ss; \
    ss.imbue(loc); \
    ss.str(to_correct_string<CharType>(actual,loc)); \
    type v; \
    ss >> manip >> v; \
    TESTEQ(v,expected); \
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
    TEST_FP2(as::number,std::setprecision(3),1200.1,"1,200.100",double,1200.1);
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
