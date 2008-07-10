#include <iostream>
#include <memory>
#include "blog.h"
#include <cppcms/manager.h>
#include <cppcms/url.h>
#include <dbixx/dbixx.h>
#include <tmpl/transtext.h>

using namespace std;
using namespace cppcms;

tmpl::template_data global_template;
transtext::trans_gnu gnugt;
transtext::trans_factory tr;

int main(int argc,char **argv)
{
	try{
		manager app(argc,argv);

		global_template.load(app.config.sval("templates.file"));

		if(app.config.lval("locale.gnugettext",0)==1) {
			gnugt.load(app.config.sval("locale.default","").c_str(),
				   "cppblog",
				   app.config.sval("locale.dir","./locale").c_str());
		}
		else {
			tr.load(app.config.sval("locale.supported","en").c_str(),
				"cppblog",
				app.config.sval("locale.dir","./locale").c_str());
		}

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
