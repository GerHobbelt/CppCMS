#include <boost/locale/generator.hpp>
#include <boost/locale/message.hpp>

namespace bl = boost::locale;

template<typename Char>
void strings_equal(std::string original,std::string iexpected,std::locale const &l,std::string domain)
{
    typedef std::basic_string<Char> string_type;
    string_type expected=to_correct_string<Char>(iexpected);
    if(domain=="default") {
        TEST(bl::translate(original).str<Char>(l)==expected);
        TEST(bl::translate(original.c_str()).str<Char>(l)==expected);
        std::locale tmp_locale=std::locale();
        std::locale::global(l);
        tmp=bl::translate(original);
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
        g.add_messages_domain("additional");
        g.add_messages_path("./");
        
        std::locale lh1=g("he_IL.UTF-8");
        std::locale lh2=g("he_IL.ISO-8859-8");
        std::locale le=g("en_US.UTF-8");

        test_translate("foo","foo",lh1,"default");
        test_translate("foo","foo",lh1,"undefined");
        test_translate("#xx#foo","foo",lh1,"undefined");
        test_translate("##foo","#foo",lh1,"undefined");
    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Ok" << std::endl;
    return EXIT_SUCCESS;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
