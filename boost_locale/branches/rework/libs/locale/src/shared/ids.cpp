#define BOOST_LOCALE_SOURCE
#include <boost/locale.hpp>

namespace boost {
    namespace locale {

        std::locale::id info::id;

        std::locale::id converter<char>::id;
        std::locale::id base_message_format<char>::id;

        #ifndef BOOST_NO_STD_WSTRING

        std::locale::id converter<wchar_t>::id;
        std::locale::id base_message_format<wchar_t>::id;

        #endif

        #ifdef BOOST_HAS_CHAR16_T

        std::locale::id converter<char16_t>::id;
        std::locale::id base_message_format<char16_t>::id;

        #endif

        #ifdef BOOST_HAS_CHAR32_T

        std::locale::id converter<char32_t>::id;
        std::locale::id base_message_format<char32_t>::id;

        #endif

        namespace boundary {        

            std::locale::id boundary_indexing<char>::id;

            #ifndef BOOST_NO_STD_WSTRING
            std::locale::id boundary_indexing<wchar_t>::id;
            #endif

            #ifdef BOOST_HAS_CHAR16_T
            std::locale::id boundary_indexing<char16_t>::id;
            #endif

            #ifdef BOOST_HAS_CHAR32_T
            std::locale::id boundary_indexing<char32_t>::id;
            #endif
        }

        namespace {
            struct install_all {
                install_all()
                {
                    std::locale l = std::locale::classic();
                    install_by<char>();
                    #ifndef BOOST_NO_STD_WSTRING
                    install_by<wchar_t>();
                    #endif
                    #ifdef BOOST_HAS_CHAR16_T
                    install_by<char16_t>();
                    #endif
                    #ifdef BOOST_HAS_CHAR32_T
                    install_by<char32_t>();
                    #endif

                    std::has_facet<info>(l);
                }
                template<typename Char>
                void install_by()
                {
                    std::locale l = std::locale::classic();
                    std::has_facet<boundary::boundary_indexing<Char> >(l);
                    std::has_facet<converter<Char> >(l);
                    std::has_facet<base_message_format<Char> >(l);
                }
            } installer;
        }

    }
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
