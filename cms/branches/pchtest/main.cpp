#include "pch.h"
#include "blog.h"

using namespace std;

tmpl::template_data global_template;

int main(int argc,char **argv)
{
	try{
		global_config.load(argc,argv);
		global_template.load(global_config.sval("templates.file"));

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
