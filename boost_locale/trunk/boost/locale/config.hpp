//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_CONFIG_HPP_INCLUDED
#define BOOST_LOCALE_CONFIG_HPP_INCLUDED

#include <boost/config.hpp>

#ifdef BOOST_HAS_DECLSPEC 
#   if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_LOCALE_DYN_LINK)
#       ifdef BOOST_LOCALE_SOURCE
#           define BOOST_LOCALE_DECL __declspec(dllexport)
#       else
#           define BOOST_LOCALE_DECL __declspec(dllimport)
#       endif  // BOOST_LOCALE_SOURCE
#   endif  // DYN_LINK
#endif  // BOOST_HAS_DECLSPEC

#ifndef BOOST_LOCALE_DECL
#   define BOOST_LOCALE_DECL
#endif


#endif // boost/locale/config.hpp
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

