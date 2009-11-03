#include <boost/locale.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <iostream>

#include <ctime>


#ifndef BOOST_NO_SWPRINTF    
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
    
    wcout<<L"Correct case conversion can't be done by simple, character by character conversion"<<endl;
    wcout<<L"because case conversion is context sensitive and not 1-to-1 conversion"<<endl;
    wcout<<L"For example:"<<endl;
    wcout<<L"   German grüßen correctly converted to "<<to_upper(L"grüßen")<<L", instead of incorrect "
                    <<boost::to_upper_copy(std::wstring(L"grüßen"))<<endl;
    wcout<<L"     where ß is replaced with SS"<<endl;
    wcout<<L"   Greek ὈΔΥΣΣΕΎΣ is correctly converted to "<<to_lower(L"ὈΔΥΣΣΕΎΣ")<<L", instead of incorrect "
                    <<boost::to_lower_copy(std::wstring(L"ὈΔΥΣΣΕΎΣ"))<<endl;
    wcout<<L"     where Σ is converted to σ or to ς, according to position in the word"<<endl;
    wcout<<L"Such type of conversion just can be done using std::toupper that work on character base, also std::toupper is "<<endl;
    wcout<<L"not fully applicable when working with variable character length like in UTF-8 or UTF-16 limiting the correct "<<endl;
    wcout<<L"behavoir to BMP or ASCII only"<<endl;
   
}
#else
int main()
{
    std::cout<<"This platform does not support wcout"<<std::endl;
}
#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
