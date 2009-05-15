#ifndef CPPCMS_HTTP_URLENCODED_H
#define CPPCMS_HTTP_URLENCODED_H
#include <cppcms/defs.h>
#include <string>

namespace cppcms { namespace util {

	namespace urlencode {
	
		std::string CPPCMS_ABI encode(char const *begin,char const *end);
		std::string CPPCMS_ABI decode(char const *begin,char const *end);
		std::string CPPCMS_ABI encode(std::string const &s);
		std::string CPPCMS_ABI decode(std::string const &s);

	} // urlencode


}} // cppcms::util

#endif
