#include <boost/locale.hpp>
#include <stdexcept>

#define CHECK(X) do { if(!(X)) throw std::runtime_error(#X); } while(0)

template<typename CharType>
struct strings
{
	static std::basic_string<CharType> widen(char const *str)
	{
		std::basic_string<CharType> tmp;
		while(*str) {
			tmp+=CharType(*str++);
		}
		return tmp;
	}
}


template<typename CharType>
void test()
{
	using namespace boost::locale;
	boost::locale::generator gen;
	
	typedef strings<CharType> st;
	typedef std::basic_stringstream<CharType> stream;
	typedef std::basic_string<CharType> str;

	std::locale l=gen("en_US.UTF-8");
	{
		stream ss;
		ss.imbue(l);
		ss << as::number << 1024.45;
		CHECK(ss.str()==st::widen("1,024.45"));
		ss.str(st::widen(""));
		ss << as::posix << 1024.45;
		CHECK(ss.str()==st::widen("1024.45"));
	}
	{
		stream ss;
		ss.imbue(l);
		ss << st::widen("10,20");
		int ten=-1,twenty=-1;
		CharType comma=0;
		ss >> ten >> comma >> twenty;
		CHECK(ss.eof());
		CHECK(!ss.fail());
		CHECK(ten==10);
		CHECK(twenty==20);
		CHECK(comma==',');

	}
	{
		stream ss;
		ss.imbue(l);
	}


}


int main()
{
	try {
		std::cout<<"char"<<std::endl;
		test<char>();
		#ifndef BOOST_NO_STD_WSTRING
		std::cout<<"wchar_t"<<std::endl;
		test<wchar_t>();
		#endif
		#ifdef BOOST_HAS_CHAR16_T
		std::cout<<"char16_t"<<std::endl;
		test<char16_t>();
		#endif
		#ifdef BOOST_HAS_CHAR32_T
		std::cout<<"char32_t"<<std::endl;
		test<char32_t>();
		#endif
	}
	catch(std::exception const &e)  {
		std::cerr<<"Failed:" << e.what() <<std::endl;
		return 1;
	}


}
