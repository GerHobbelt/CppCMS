#define BOOST_LOCALE_SOURCE
#include <boost/locale.hpp>

namespace boost {
	namespace locale {

		std::locale::id converter<char>::id;
        	#ifndef BOOST_NO_STD_WSTRING
		std::locale::id converter<wchar_t>::id;
		#endif
        	#ifdef BOOST_HAS_CHAR16_T
		std::locale::id converter<char16_t>::id;
		#endif
        	#ifdef BOOST_HAS_CHAR32_T
		std::locale::id converter<char32_t>::id;
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

	}
}
