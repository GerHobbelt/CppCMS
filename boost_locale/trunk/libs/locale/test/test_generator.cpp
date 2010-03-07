#include <boost/locale/generator.hpp>
#include <boost/locale/collator.hpp>
#include <iomanip>
#include "test_locale.hpp"


int main()
{
    try {
        boost::locale::generator g;
        std::locale l=g("en_US.UTF-8");
        TEST(std::has_facet<boost::locale::collator<char> >(l));

        g.categories(g.categories() ^ boost::locale::collation_facet);
        g.preload("en_US.UTF-8");
        g.categories(g.categories() | boost::locale::collation_facet);
        l=g("en_US.UTF-8");
        TEST(!std::has_facet<boost::locale::collator<char> >(l));
        g.clear_cache();
        l=g("en_US.UTF-8");
        TEST(std::has_facet<boost::locale::collator<char> >(l));
        g.characters(g.characters() ^ boost::locale::char_facet);
        l=g("en_US.UTF-8");
        TEST(!std::has_facet<boost::locale::collator<char> >(l));
        g.characters(g.characters() | boost::locale::char_facet);
        l=g("en_US.UTF-8");
        TEST(std::has_facet<boost::locale::collator<char> >(l));
    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    FINALIZE();

}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
