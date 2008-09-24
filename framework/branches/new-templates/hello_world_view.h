#include "base_view.h"

class my_hello_world;
namespace view {

struct cppcms_master_view(master,my_hello_world) {
	virtual void render() { 
		cout() <<"<html><body dir=\"rtl\">"<<std::endl;
		content();
		cout() <<"</body></html>"<<std::endl;
	};
	virtual void content() {};
};

struct cppcms_extend_view(hello,master) {
	std::string msg;
	virtual void content() { cout() << msg ; }
};

};

