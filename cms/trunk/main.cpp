#include <iostream>
#include <memory>
#include "blog.h"
#include <cppcms/thread_pool.h>
#include <cppcms/global_config.h>
#include <cppcms/url.h>
#include <dbi/dbixx.h>
#include <cppcms/templates.h>
#include <tmpl/transtext.h>

using namespace std;

tmpl::template_data global_template;
transtext::trans_factory tr;

int main(int argc,char **argv)
{
	try{
		global_config.load(argc,argv);
		global_template.load(global_config.sval("templates.file"));

		tr.load(global_config.sval("locale.supported","en").c_str(),
			"cppblog",
			global_config.sval("locale.dir","./locale").c_str());

		Run_Application<Blog>(argc,argv);

		cout<<"Exiting\n";
	}
	catch(HTTP_Error &s) {
		cerr<<s.get()<<endl;
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
