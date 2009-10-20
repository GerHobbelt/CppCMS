#include <iostream>
#include <locale>
#include <boost/locale/formatting.hpp>
#include <boost/locale/numeric.hpp>
#include <boost/locale/info.hpp>
#include <iomanip>
#include <sstream>

int main()
{
	using namespace boost::locale;
	std::locale base("");
	std::locale::global(base);
	std::locale base1(base,new info(getenv("LC_ALL")));
	std::locale base2(base1,new num_format<wchar_t>());

	std::wcout.imbue(base2);
	std::wcout<<std::setw(20)<<L"שלום עולם!!!"<<std::endl;
	std::wcout<<as::number<<104<<" "<<1234.56<<std::endl;
	std::wcout<<std::scientific<<104<<" "<<1234.56<<std::endl;
	std::wcout<<std::setprecision(5)<<104<<" "<<1234.56<<std::endl;
	std::wcout<<as::percent<<104<<" "<<1234.56<<std::endl;
	std::wcout<<as::currency<<104<<" "<<1234.56<<std::endl;
	std::wcout<<as::currency_iso<<104<<" "<<1234.56<<std::endl;
	std::wcout<<as::spellout<<104<<" "<<1234.56<<std::endl;
	std::wcout<<as::ordinal<<104<<" "<<1234.56<<std::endl;
}
