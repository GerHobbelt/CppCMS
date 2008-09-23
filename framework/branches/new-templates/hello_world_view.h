#include "base_view.h"

class my_hello_world;
namespace view {
struct master: virtual public cppcms::base_view<my_hello_world> {
	virtual void render() { 
		cout() <<"<html><body dir=\"rtl\">"<<std::endl;
		content();
		cout() <<"</body></html>"<<std::endl;
	};
	virtual void content() {};
};

struct hello: virtual public master {
	std::string msg;
	virtual void content() { cout() << msg ; }
};

};

