#include <boost/locale.hpp>
#include <iomanip>
#include <ctime>

int main(int argc,char **argv)
{
	using namespace boost::locale;
	generator gen;
	gen.octet_encoding("UTF-8");

	/// Setup environment

	if(argc>=2)
		std::locale::global(gen(argv[1]));
	else
		std::locale::global(gen("")); // system default

	boost::locale::date_time now(std::time(0));
	
	date_time start=now;

	/// Set the minimum month and day for this year
	start.set(period::month,now.minimum(period::month));
	start.set(period::day,start.minimum(period::day));

	int current_year = now / period::year;

	std::cout.imbue(std::locale());
	std::cout << format("{1,ftime='%Y'}") % now << std::endl;
	for(now=start;now / period::year == current_year;) {
		
		/// Print heading of month
		if(calendar().is_gregorian()) 
			std::cout << format("{1,ftime='%B'}") % now <<std::endl;
		else
			std::cout << format("{1,ftime='%B'} ({1,date=s,locale=en,date=s} - {2,locale=en,date=s})")
					% now 
					% date_time(now,now.maximum(period::day)*period::day) << std::endl;

		int first = calendar().first_day_of_week();

		/// Print weeks days
		for(int i=0;i<7;i++) {
			date_time tmp(now,period::day_of_week * (first + i));
			std::cout << format("{1,w=8,ftime='%a'} ") % tmp;
		}
		std::cout << std::endl;

		int current_month = now / period::month;
		int skip = now / period::day_of_week_local - 1;
		for(int i=0;i<skip*9;i++)
			std::cout << ' ';
		for(;now / period::month == current_month ;now += period::day) {
			std::cout << format("{1,w=8,ftime='%e'} ") % now;	
			if(now / period::day_of_week_local == 7)
				std::cout << std::endl;
		}
		std::cout << std::endl;
	}

}
