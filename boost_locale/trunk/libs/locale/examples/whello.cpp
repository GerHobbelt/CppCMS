#include <boost/locale.hpp>
#include <iostream>

#include <ctime>

int main()
{
    using namespace boost::locale;
    using namespace std;
    generator gen;
    locale::global(locale(""));
    locale loc=gen(""); 
    // Create system default locale

    locale::global(loc); 
    // Make it system global
    
    wcout.imbue(loc);
    // Set as default locale for output
    
    wcout <<wformat(L"Today {1,date} at {1,time} we had run our first localization example") % time(0) 
          <<endl;
   
    wcout<<L"This is how we show numbers in this locale "<<as::number << 103.34 <<endl; 
    wcout<<L"This is how we show currency in this locale "<<as::currency << 103.34 <<endl; 
    wcout<<L"This is typical date in the locale "<<as::date << std::time(0) <<endl;
    wcout<<L"This is typical time in the locale "<<as::time << std::time(0) <<endl;
    wcout<<L"This is upper case "<<to_upper(L"Hello World!")<<endl;
    wcout<<L"This is lower case "<<to_lower(L"Hello World!")<<endl;
    wcout<<L"This is title case "<<to_title(L"Hello World!")<<endl;
    wcout<<L"This is fold case "<<fold_case(L"Hello World!")<<endl;
   
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
