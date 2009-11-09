//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <iostream>
#include <string>
#include <set>

#include <boost/locale.hpp>

using namespace std;
using namespace boost::locale;

int main(int argc,char **argv)
{
	if(argc!=3) {
		cerr<<"Usage collate Locale-ID Encoding"<<endl;
		return 1;
	}
	generator gen;
	
	std::locale::global(gen.generate(argv[1],argv[2]));

	typedef std::set<std::string,comparator<char> > set_type;
	set_type all_strings;
	while(!cin.eof()) {
		std::string tmp;
		getline(cin,tmp);
		all_strings.insert(tmp);
	}
	for(set_type::iterator p=all_strings.begin();p!=all_strings.end();++p) {
		cout<<*p<<endl;
	}

}
