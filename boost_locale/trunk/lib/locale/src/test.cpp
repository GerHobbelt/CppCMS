#include <iostream>
#include <locale>
#include <boost/locale/formatting.hpp>
#include <boost/locale/numeric.hpp>
#include <boost/locale/info.hpp>
#include <boost/locale/message.hpp>
#include <boost/locale/collator.hpp>
#include <iomanip>
#include <sstream>
#include <ctime>

int main()
{
	try {
	using namespace boost::locale;
	using namespace std;
	std::locale base;
	try {
		base=std::locale("");
	}
	catch(std::exception const &e)
	{}
	std::locale::global(base);
	base = std::locale(base,new info(getenv("LC_ALL")));
	base = std::locale(base,new num_format<char>());
	base = std::locale(base,new num_parse<char>());
	base = std::locale(base,collator<char>::create(std::use_facet<info>(base)));


	messages_loader loader;
	loader.add_path(".");
	loader.domain("test");

	std::cout << std::use_facet<collator<char> >(base).compare(collator_base::primary,"hello","Hello") << std::endl;
	std::cout << std::use_facet<collator<char> >(base).compare(collator_base::tertiary,"hello","Hello") << std::endl;

	base=loader.load(base);
	
	stringstream out;
	out.imbue(base);

	time_t now=std::time(0);

	out<<as::number<<104<<" "<<1234<<" ";
	out<<std::scientific<<1234.56e-10<<std::endl;
	out<<as::currency<<104.12<<" ";
	out<<as::currency_iso<<104.12<<endl;
	out<<as::spellout<<104<<"   "<<1234.56<<std::endl;
	out<<as::ordinal<<104<<endl;
	out<<as::date<<now<<endl;
	out<<as::datetime<<as::date_full<<as::time_full << std::time(0) << endl;

	std::cout<<out.str()<<std::flush;
	out<<translate("#A message#Hello World")<<endl;
	out<<translate("##Hello World")<<endl;
	

	int v_104 = 0,v_1234 = 0;
	time_t now_a = 0;
	double v_1234_56e_10 = 0,v_1234_56 = 0,v_104_12 = 0;

	
	out.unsetf(std::ios_base::scientific);
	out >> as::date_default >> as::time_default;

	out>> as::number >> v_104 >> v_1234;

	std::cout<<"104 = "<<104<<" 1234 = "<<v_1234<<endl;
	out>>std::scientific>>v_1234_56e_10;
	cout << "v_1234_56e_10="<<v_1234_56e_10 << endl;

	out>>as::currency>>v_104_12;
	cout << "v_104_12 = " << v_104_12 <<endl;
	v_104_12=0;
	out>>as::currency_iso>>v_104_12;
	
	cout << "v_104_12 = " << v_104_12 <<endl;
	v_104=0;
	v_1234_56 = 0;

	out>>as::spellout >> v_104 >> v_1234_56;
	cout << "v_104 = " << v_104 <<endl;
	cout << "v_1034.56 = " << v_1234_56 <<endl;

	v_104=0;
	out >> as::ordinal >> v_104 ;
	cout << "v_104 = " << v_104 <<endl;

	out>>as::date>>now_a;
	cout << now_a <<"="<< now <<endl;
	now_a=0;
	out>>as::datetime>>as::date_full>>as::time_full >> now_a;
	cout << now_a <<"="<< now <<endl;

	std::cout<<out.str();
	}
	catch(std::exception const &e) {
	     std::cerr<<e.what()<<std::endl;
	}
	
}
