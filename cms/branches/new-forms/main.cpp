#include <iostream>
#include <cppcms/manager.h>
#include <dbixx/dbixx.h>

#include "blog.h"

using namespace std;
using namespace cppcms;

int main(int argc,char **argv)
{
	try{
		manager app(argc,argv);
		app.set_worker(new simple_factory<Blog>());
		app.execute();
	}
	catch(cppcms_error &s) {
		cerr<<s.what()<<endl;
		return 1;
	}
	catch(dbixx::dbixx_error &e) {
		cerr<<"dbi:"<<e.what()<<endl;
	}
	catch(std::exception &e) {
		cerr<<e.what()<<endl;
	}
	return 0;
}
