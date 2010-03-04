#include "test_locale.hpp"
#include "../src/ios_prop.hpp"
#include <sstream>
#include <locale>

int counter=0;
int imbued=0;
struct propery {
	propery(int xx=-1) : x(xx) { counter ++; }
	propery(propery const &other) { counter++; x=other.x; }
	propery const &operator=(propery const &other) {
		x=other.x;
		return *this;
	};
	int x;
	void on_imbue() {imbued++; }
	~propery() { counter--; }
};
typedef boost::locale::impl::ios_prop<propery> prop_type;

struct init {
	init() { prop_type::global_init(); }
};

int main()
{
	try {
		{
			std::stringstream ss;
			TEST(!prop_type::has(ss));
			TEST(prop_type::get(ss).x==-1);
			TEST(prop_type::has(ss));
			TEST(counter==1);
		}
		TEST(counter==0);
		{
			std::stringstream ss;
			prop_type::set(propery(1),ss);
			TEST(counter==1);
			TEST(prop_type::get(ss).x==1);
		}
		TEST(counter==0);
		{
			std::stringstream ss;
			prop_type::set(propery(1),ss);
			TEST(counter==1);
			TEST(prop_type::get(ss).x==1);
		}
		TEST(counter==0);
		{
			std::stringstream ss,ss2;
			prop_type::set(propery(2),ss);
			ss2.copyfmt(ss);
			TEST(prop_type::get(ss).x==2);
			TEST(prop_type::has(ss2));
			TEST(prop_type::has(ss));
			TEST(prop_type::get(ss2).x==2);
			prop_type::get(ss2).x=3;
			TEST(prop_type::get(ss2).x==3);
			TEST(prop_type::get(ss).x==2);
			TEST(counter==2);
			TEST(imbued==0);
			ss2.imbue(std::locale::classic());
			TEST(imbued==1);
		}
		TEST(counter==0);
	}catch(std::exception const &e) {
		std::cerr << "Fail" << std::endl;
		return EXIT_FAILURE;
	}
	FINALIZE();
	return 0;
}

