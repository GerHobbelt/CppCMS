#include <iostream>
#include <memory>
#include "blog.h"
#include <cppcms/thread_pool.h>
#include <cppcms/global_config.h>
#include <cppcms/url.h>

#include <cppcms/templates.h>
#include "data.h"

using namespace std;

Templates_Set templates;

int main(int argc,char **argv)
{
	try{
		global_config.load(argc,argv);

		db_initall();
		db_openall();

		templates.load();

		Run_Application<Blog>(argc,argv);

		db_closeall();
		cout<<"Exiting\n";
	}
	catch(HTTP_Error &s) {
		cerr<<s.get()<<endl;
		db_closeall();
		return 1;
	}
	catch(DbException &e) {
		cerr<<e.what()<<endl;
	}
	catch(std::exception &e) {
		cerr<<e.what()<<endl;
	}
	return 0;
}
