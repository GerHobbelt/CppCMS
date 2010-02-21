#ifndef BOOST_LOCALE_TEST_H
#define BOOST_LOCALE_TEST_H
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <unicode/utf8.h>

#define TEST(X)                                                 \
    do {                                                        \
        if(X) break;                                            \
        std::ostringstream oss;                                 \
        oss << "Error " << __FILE__ << ":"<<__LINE__ << " "#X;  \
        throw std::runtime_error(oss.str());                    \
    }while(0)    


#endif

template<typename Char>
std::basic_string<Char> to(std::string const &utf8)
{
	std::basic_string<Char> out;
	unsigned i=0;
	while(i<utf8.size()) {
		unsigned point;
		unsigned prev=i;
		U8_NEXT_UNSAFE(utf8,i,point);
		if(sizeof(Char)==1 && point > 255) {
			std::ostringstream ss;
			ss << "Can't convert codepoint U" << std::hex << point <<"(" <<std::string(utf8.begin()+prev,utf8.begin()+i)<<") to Latin1";
			throw std::runtime_error(ss.str());
		}
		else if(sizeof(Char)==2 && point >0xFFFF) { // Deal with surragates
	        point-=0x10000;
	        out+=static_cast<Char>(0xD800 | (point>>10));
	        out+=static_cast<Char>(0xDC00 | (point & 0x3FF));
	        continue;
		}
		out+=static_cast<Char>(point);
	}
	return out;
}


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
