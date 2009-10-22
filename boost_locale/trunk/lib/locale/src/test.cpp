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
	using namespace std;
	std::locale base("en_US.UTF-8");
	std::locale::global(base);
	std::locale base1(base,new info(getenv("LC_ALL")));
	std::locale base2(base1,new num_format<char>());

	std::cout.imbue(base2);
	std::cout<<as::number<<104<<" "<<1234.56<<std::endl;
	std::cout<<std::scientific<<104<<" "<<1234.56<<std::endl;
	std::cout<<std::setprecision(5)<<104<<" "<<1234.56<<std::endl;
	std::cout<<setw(10)<<right<<as::percent<<0.34<<endl;//<<" "<<1234.56<<std::endl;
	std::cout<<as::currency<<104<<" "<<1234.56<<std::endl;
	std::cout<<as::currency_iso<<104<<" "<<1234.56<<std::endl;
	std::cout<<as::spellout<<104<<" "<<1234.56<<std::endl;
	std::cout<<as::ordinal<<104<<" "<<1234.56<<std::endl;
}
