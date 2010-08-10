#include "iconv_codepage.hpp"
#include "uconv_codepage.hpp"
#include "wconv_codepage.hpp"

#include <fstream>
#include <memory>

int main(int argc,char **argv)
{
	std::string utf8;
	std::string cp1255;
	std::wstring utf16;

	std::ifstream file("test.txt");
	if(!file) {
		return 1;
	}
	char buf[1024];
	while(!file.eof()) {
		file.read(buf,1024);
		utf8.append(buf,file.gcount());
	}
	file.close();

	using namespace boost::locale::conv;
	using namespace boost::locale::conv::impl;

	std::auto_ptr<converter_between> between;
	std::auto_ptr<converter_to_utf<wchar_t> > to_utf16;
	std::auto_ptr<converter_from_utf<wchar_t> > from_utf16;

	if(argc!=2) {
		std::cerr <<"Usage [iuw][tfb]" << std::endl;
		return 1;
	}
	char c1=argv[1][0];
	char c2=argv[1][1];
	if(c1=='i') {
		between.reset(new iconv_between());
		to_utf16.reset(new iconv_to_utf<wchar_t>());
		from_utf16.reset(new iconv_from_utf<wchar_t>());
	}
	else if(c1=='u') {
		between.reset(new uconv_between());
		to_utf16.reset(new uconv_to_utf<wchar_t>());
		from_utf16.reset(new uconv_from_utf<wchar_t>());
	}
	else if(c1=='w') {
		between.reset(new wconv_between());
		to_utf16.reset(new wconv_to_utf<wchar_t>());
		from_utf16.reset(new wconv_from_utf<wchar_t>());
	}
	else {
		assert(0);
	}
	assert(between->open("windows-1255","utf-8",stop));
	assert(to_utf16->open("utf-8",stop));
	assert(from_utf16->open("windows-1255",stop));
	
	cp1255=between->convert(utf8.c_str(),utf8.c_str()+utf8.size());
	utf16=to_utf16->convert(utf8.c_str(),utf8.c_str()+utf8.size());

	int times=100000;

	if(c2=='t') {
		for(int i=0;i<times;i++) {
			assert(to_utf16->convert(utf8.c_str(),utf8.c_str()+utf8.size()) == utf16);
		}
	}
	else if(c2=='f') {
		for(int i=0;i<times;i++) {
			assert(from_utf16->convert(utf16.c_str(),utf16.c_str()+utf16.size()) == cp1255);
		}
	}
	else if(c2=='b') {
		for(int i=0;i<times;i++) {
			assert(!between->convert(utf8.c_str(),utf8.c_str()+utf8.size()).empty());
		}
	}
	else {
		assert(0);
	}
	

}
