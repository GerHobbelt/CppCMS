#include <iostream>
#include <memory>
#include "blog.h"
#include <cppcms/thread_pool.h>
#include <cppcms/global_config.h>
#include <cppcms/url.h>
#include <soci/soci.h>
#include <cppcms/templates.h>

using namespace std;

Templates_Set templates;

int main(int argc,char **argv)
{
	try{
		global_config.load(argc,argv);

		templates.load();

		Run_Application<Blog>(argc,argv);

		cout<<"Exiting\n";
	}
	catch(HTTP_Error &s) {
		cerr<<s.get()<<endl;
		return 1;
	}
	catch(soci::soci_error &e) {
		cerr<<"soci:"<<e.what()<<endl;
	}
	catch(std::exception &e) {
		cerr<<e.what()<<endl;
	}
	return 0;
}
