#include <boost/locale/message.hpp>

namespace boost {
namespace locale {

    template<typename CharType>
    struct messages_info {
        std::string language;
        std::string country;
        std::string variant;
        std::string encoding;
        std::vector<std::string> domains;
        std::vector<std::string> paths;
        virtual bool set_encoding(std::string const &input_encoding) = 0;
        virtual std::basic_string<CharType> convert(char const *begin,char const *end) = 0;
        virtual ~messages_info()
        {
        }
    };

    template<typename CharType>
    message_format<CharType> *create_messages_facet(messages_info<CharType> &info);
    
    template<>
    message_format<char> *create_messages_facet(messages_info<char> &info);
    

    #ifndef BOOST_NO_STD_WSTRING
    template<>
    message_format<wchar_t> *create_messages_facet(messages_info<wchar_t> &info);
    #endif

    #ifdef BOOST_HAS_CHAR16_T
    template<>
    message_format<char16_t> *create_messages_facet(messages_info<char16_t> &info);
    #endif
    
    #ifdef BOOST_HAS_CHAR32_T
    template<>
    message_format<char32_t> *create_messages_facet(messages_info<char32_t> &info);
    #endif


} // locale
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

