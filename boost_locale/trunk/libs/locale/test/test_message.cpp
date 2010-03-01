#include <boost/locale/generator.hpp>
#include <boost/locale/message.hpp>
#include "test_locale.hpp"
#include "test_locale_tools.hpp"

namespace bl = boost::locale;

#define BOOST_NO_STD_WSTRING

template<typename Char>
void strings_equal(std::string original,std::string iexpected,std::locale const &l,std::string domain)
{
    typedef std::basic_string<Char> string_type;
    string_type expected=to_correct_string<Char>(iexpected,l);
    if(domain=="default") {
        TEST(bl::translate(original).str<Char>(l)==expected);
        TEST(bl::translate(original.c_str()).str<Char>(l)==expected);
        std::locale tmp_locale=std::locale();
        std::locale::global(l);
        string_type tmp=bl::translate(original);
        TEST(tmp==expected);
        tmp=bl::translate(original).str<Char>();
        TEST(tmp==expected);
        std::locale::global(tmp_locale);

        std::basic_ostringstream<Char> ss;
        ss.imbue(l);
        ss << bl::translate(original);
        TEST(ss.str()==expected);
    }
    TEST(bl::translate(original).str<Char>(l,domain)==expected);
    std::locale tmp_locale=std::locale();
    std::locale::global(l);
    TEST(bl::translate(original).str<Char>(domain)==expected);
    std::locale::global(tmp_locale);
    {
        std::basic_ostringstream<Char> ss;
        ss.imbue(l);
        ss << bl::as::domain(domain) << bl::translate(original);
        TEST(ss.str()==expected);
    }
    {
        std::basic_ostringstream<Char> ss;
        ss.imbue(l);
        ss << bl::as::domain(domain) << bl::translate(original.c_str());
        TEST(ss.str()==expected);
    }
}

void test_translate(std::string original,std::string expected,std::locale const &l,std::string domain)
{
    strings_equal<char>(original,expected,l,domain);
    #ifndef BOOST_NO_STD_WSTRING
    strings_equal<wchar_t>(original,expected,l,domain);
    #endif
    #ifdef BOOST_HAS_CHAR16_T
    strings_equal<char16_t>(original,expected,l,domain);
    #endif
    #ifdef BOOST_HAS_CHAR32_T
    strings_equal<char32_t>(original,expected,l,domain);
    #endif
    
}


int main()
{
    try {
        boost::locale::generator g;
        g.add_messages_domain("default");
        g.add_messages_domain("simple");
        g.add_messages_path("./");
        
        std::string locales[] = { "he_IL.UTF-8", "he_IL.ISO-8859-8" };

        for(int i=0;i<sizeof(locales)/sizeof(locales[0]);i++){
            std::locale l=g(locales[i]);
            
            std::cout << "Testing "<<locales[i]<<std::endl;
            std::cout << " single forms" << std::endl;

            test_translate("hello","שלום",l,"default");
            test_translate("hello","היי",l,"simple");
            test_translate("hello","hello",l,"undefined");
            test_translate("untranslated","untranslated",l,"default");
            test_translate("##untranslated","#untranslated",l,"default");
            test_translate("#xx#untranslated","untranslated",l,"default");
            test_translate("##hello","#hello",l,"undefined");
            test_translate("#xx#hello","hello",l,"undefined");
            test_translate("#context#hello","שלום בהקשר אחר",l,"default");
            test_translate("##hello","#שלום",l,"default");
        }
    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Ok" << std::endl;
    return EXIT_SUCCESS;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
