#include <unicode/locid.h>

namespace boost {
namespace locale {
namespace impl_icu { 
    class icu_localization_backend : public localization_backend {
    public:
        icu_localization_backend() {}
        icu_localization_backend(icu_localization_backend const &other) : 
            locale_id_(other.locale_id_),
            domains_(other.domains_),
            paths_(other.paths_)
        {
        }
        virtual icu_localization_backend *clone()
        {
            return new icu_localization_backend(*this);
        }

        void set_option(std::string const &name,std::string const &value) 
        {
            if(name=="locale")
                locale_id_ = value;
            else if(name=="message_path")
                paths_.push_back(value);
            else if(name=="message_application")
                domains_.push_back(value);
        }
        void clear_options()
        {
            locale_id_.clear();
            paths_.clear();
            domains_.clear();
        }

        std::string encoding_from_locale(std::string const &str)
        {
            std::string lenc;
            size_t start = str.find('.');
            if(start!=std::string::npos) {
                start++;
                size_t end = str.find('@',start);
                lenc=str.substr(start,end-start);
            }
            if(lenc.empty()) {
                lenc=ucnv_getDefaultName();
            }
            if(ucnv_compareNames(lenc.c_str(),"UTF8")==0) {
                lenc="UTF-8";
            }
            return lenc;
        }
        
        virtual std::locale install(std::locale const &base,
                                    locale_category_type category,
                                    character_facet_type type = nochar_facet)
        {
            cdata d;
            d.locale = icu::Locale::createCanonical(locale_id_.c_str());
            d.encoding = encoding_from_locale(locale_id_);
            d.utf8 = d.encoding == "UTF-8";

            std::locale res;

            if(non_character_facet_first <= category && category <= non_character_facet_last) {
                switch(category) {
                case calendar_facet:
                    res = std::locale(base,new icu_calendar_facet(d));
                    break;
                default:
                    res = base;
                }
            }
            else if(per_character_facet_first <= category && category <= per_character_facet_last) {
                switch(type) {
                case char_facet:
                    res = install_by_char<char>(base,d,category);
                    break;
                #ifndef BOOST_NO_STD_WSTRING
                case wchar_t_facet:
                    res = install_by_char<wchar_t>(base,d,category);
                    break;
                #endif
                #ifdef BOOST_HAS_CHAR16_T
                case char_16_t_facet:
                    res = install_by_char<char16_t>(base,d,category);
                    break;
                #endif
                #ifdef BOOST_HAS_CHAR32_T
                case char_32_t_facet:
                    res = install_by_char<char32_t>(base,d,category);
                    break;
                #endif
                default:
                    res = base;
                }
            }
            else {
                res = base;
            }
            return res;
        }
    private:
        template<typename CharType>
        std::locale install_by_char(std::locale const &base,cdata const &d,locale_category_type cat)
        {
            std::locale res;
            switch(cat) {
            case convert_facet:
                res = std::locale(base,new converter_impl<CharType>(d));
                break;
            case collation_facet:
                break;
            case formatting_facet:
                break;
            case parsing_facet:
                break;
            case message_facet:
                break;
            case codepage_facet:
                break;
            case boundary_facet:
                break;
            default:
                res = base;
            }
            return res;
        }
        std::vector<std::string> paths_;
        std::vector<std::string> domains_;
        std::string locale_id_;
    };

}  // impl icu
}  // locale
}  // boost
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
