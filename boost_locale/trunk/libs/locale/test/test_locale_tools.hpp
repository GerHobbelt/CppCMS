#ifndef BOOST_LOCLAE_TEST_LOCALE_TOOLS_HPP
#define BOOST_LOCLAE_TEST_LOCALE_TOOLS_HPP

#include <boost/locale/codepage.hpp>

template<typename Char>
std::basic_string<Char> to_correct_string(std::string const &e,std::locale l)
{
    return boost::locale::conv::to_utf<Char>(e,l);
}


template<>
inline std::string to_correct_string(std::string const &e,std::locale l)
{
    return boost::locale::conv::from_utf(e,l);
}

#endif
