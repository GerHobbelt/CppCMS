#include <boost/locale/codepage.hpp>
#include <boost/locale/generator.hpp>
#include <boost/locale/info.hpp>
#include <fstream>
#include "test.h"

template<typename Char>
std::basic_string<Char> read_file(std::basic_istream<Char> &in)
{
    std::basic_string<Char> res;
    Char c;
    while(in.get(c))
        res+=c;
    return res;
}


template<typename Char>
void test_ok(std::string file,std::locale const &l)
{
    std::ofstream test("testi.txt");
    test << file;
    test.close();
    typedef std::basic_fstream<Char> stream_type;

    stream_type f1("testi.txt",stream_type::in);
    f1.imbue(l);
    std::wstring ss=read_file<Char>(f1);
    std::cout << ss.size() <<" "<< to<Char>(file).size() << std::endl;
    for(unsigned i=0;i<ss.size();i++)
        std::cout << char(ss[i]);
    std::cout << std::endl;
    TEST(ss == to<Char>(file)); 
    std::cout << "B" << std::endl;
    f1.close();

    stream_type f2("testo.txt",stream_type::out);
    f2.imbue(l);
    f2 << to<Char>(file);
    f2.close();

    std::ifstream testo("testo.txt");
    TEST(read_file<char>(testo) == file);
}

template<typename Char>
void test_rfail(std::string file,std::locale const &l,int pos)
{
    std::ofstream test("testi.txt");
    test << file;
    test.close();
    typedef std::basic_fstream<Char> stream_type;

    stream_type f1("testi.txt",stream_type::in);
    f1.imbue(l);
    Char c;
    for(int i=0;i<pos;i++)
        TEST((f1 >>c).good());
    TEST((f1 >> c).fail());
}

template<typename Char>
void test_wfail(std::string file,std::locale const &l,int pos)
{
    typedef std::basic_fstream<Char> stream_type;
    stream_type f1("testo.txt",stream_type::out);
    f1.imbue(l);
    std::basic_string<Char> out=to<Char>(file);
    Char c;
    int i;
    for(i=0;i<pos;i++) {
        f1 << out.at(i);
        f1<<std::flush;
        TEST(f1.good());
    }
    f1 << out.at(i);
    TEST(f1.fail() || (f1<<std::flush).fail());
}


template<typename Char>
void test_for_char()
{
    boost::locale::generator g;
    test_ok<Char>("grüße\nn i",std::locale("en_US.ISO-8859-1"));
    test_wfail<Char>("grüßen",std::locale("en_US.ISO-8859-1"),2);
    test_ok<Char>("grüße\nn i",std::locale("he_IL.UTF-8"));
    test_rfail<Char>("abc\xFF\xFF",std::locale("he_IL.UTF-8"),3);
    if(sizeof(Char) > 2)  {
        test_ok<Char>("abc\"\xf0\xa0\x82\x8a\"",std::locale("he_IL.UTF-8")); // U+2008A
    }
    else {
        test_rfail<Char>("abc\"\xf0\xa0\x82\x8a\"",std::locale("he_IL.UTF-8"),4);
        test_wfail<Char>("abc\"\xf0\xa0\x82\x8a\"",std::locale("he_IL.UTF-8"),4);
    }
}
void test_wide_io()
{
    #ifndef BOOST_NO_STD_WSTRING
    std::cout << "Testing wchar_t" << std::endl;
    test_for_char<wchar_t>();
    #endif
    #ifdef BOOST_HAS_CHAR16_T
    std::cout << "Testing char16_t" << std::endl;
    test_for_char<char16_t>();
    #endif
    #ifdef BOOST_HAS_CHAR32_T
    std::cout << "Testing char32_t" << std::endl;
    test_for_char<char32_t>();
    #endif
}

int main()
{
    try {
        std::cout << "Testing wide I/O" << std::endl;
        test_wide_io();
    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Ok" << std::endl;
    return EXIT_SUCCESS;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
