#include <iostream>
#include <memory>
#include "blog.h"
#include <cppcms/thread_pool.h>
#include <cppcms/global_config.h>
#include <cppcms/url.h>
#include <dbi/dbixx.h>
#include <tmpl/transtext.h>

using namespace std;
using namespace cppcms;

tmpl::template_data global_template;
transtext::trans_gnu gnugt;
transtext::trans_factory tr;

int main(int argc,char **argv)
{
	try{
		global_config.load(argc,argv);
		global_template.load(global_config.sval("templates.file"));

		if(global_config.lval("locale.gnugettext",0)==1) {
			gnugt.load(global_config.sval("locale.default","").c_str(),
				   "cppblog",
				   global_config.sval("locale.dir","./locale").c_str());
		}
		else {
			tr.load(global_config.sval("locale.supported","en").c_str(),
				"cppblog",
				global_config.sval("locale.dir","./locale").c_str());
		}

		run_application(argc,argv,simple_factory<Blog>());

		cout<<"Exiting\n";
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
