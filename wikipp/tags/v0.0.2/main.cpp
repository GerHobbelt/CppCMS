#include <cppcms/manager.h>
#include "wiki.h"

using namespace cppcms;

int main(int argc,char ** argv)
{
	try {
		manager app(argc,argv);
		app.set_worker(new application_factory<apps::wiki>());
		app.execute();
	}
	catch(std::exception const &e) {
		cerr<<e.what()<<endl;
	}
}
