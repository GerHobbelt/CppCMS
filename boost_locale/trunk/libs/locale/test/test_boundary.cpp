#include <boost/locale/boundary.hpp>
#include <boost/locale/generator.hpp>
#include "test.h"

void word_boundary()
{
    boost::locale::generator g;
    std::string all[]={"10"," ","Hello"," ","Windows7"," ","平仮名","ひらがな","ヒラガナ",""};
    int         num[]={ 1,   0,      0,  0,         1,   0,      0 ,        0 ,         0};
    int        word[]={ 0,   0,      1,  0,         1,   0,      1 ,        1 ,         1};
    int        kana[]={ 0,   0,      0,  0,         0,   0,      0,         1 ,         1}; 
    int        ideo[]={ 0,   0,      0,  0,         0,   0,      1,         0 ,         0}; 
    run_tocken<char>(all,num,word,kana,ideo,g("ja_JP.UTF-8"));
    run_tocken<char>(all,num,word,kana,ideo,g("ja_JP.Shift-JIS"));
    #ifndef BOOST_NO_STD_WSTRING
    run_tocken<wchar_t>(all,num,word,kana,ideo,g("ja_JP.UTF-8"));
    #endif 
    #ifdef BOOST_HAS_CHAR16_T
    run_tocken<char16_t>(all,num,word,kana,ideo,g("ja_JP.UTF-8"));
    #endif 
    #ifdef BOOST_HAS_CHAR32_T
    run_tocken<char32_t>(all,num,word,kana,ideo,g("ja_JP.UTF-8"));
    #endif 
}



int main()
{
    try {
    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Ok" << std::endl;
    return EXIT_SUCCESS;

}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

