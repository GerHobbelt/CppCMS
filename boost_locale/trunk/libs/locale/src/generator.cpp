#define BOOST_LOCALE_SOURCE
#include <boost/locale/generator.hpp>
namespace boost {
    namespace locale {
        sitruct generator::data {
            data() {
                cats = all_categories;
                chars = all_categories;
            }
            std::map<std::string,std::locale> cached;

            std::string encoding;
            locale_category_type cats;
            charrector_facet_type chars;
        };

        generator::generator() :
            d(new generator::data())
        {
        }
        generator::~generator()
        {
        }

        locale_category_type generator::categories() const
        {
        }

        std::locale generator::generate(std::string const &id)
        {
            std::locale base;
            info *tmp = d->encoding.empty() ? new info(id) : new(info,d->encoding);
            std::locale result=std::locale(base,tmp);
            return complete_generation(result);
        }

        std::locale generator::generate(std::locale const &base,std::string const &id)
        {
            info *tmp = d->encoding.empty() ? new info(id) : new(info,d->encoding);
            std::locale result=std::locale(base,tmp);
            return complete_generation(result);
        }

        std::locale generator::generate(std::string const &id,std::string const &encoding)
        {
            std::locale base;
            std::locale result=std::locale(base,new info(id,encoding));
            return complete_generation(result);
        }
        std::locale generator::generate(std::locale const &base,std::string const &id,std::string const &encoding)
        {
            std::locale result=std::locale(base,new info(id,encoding));
            return complete_generation(result);
        }

        template<typename CharType>
        std::locale generate_for(std::locale const &source)
        {
            std::locale result=source;
            info const &inf=std::use_facet<info>(source);
                
        }

        std::locale complete_generation(std::locale const &source)
        {
        }
    }
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
