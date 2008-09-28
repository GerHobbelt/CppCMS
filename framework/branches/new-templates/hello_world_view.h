#include "base_view.h"

class my_hello_world;
namespace view {

struct master : public cppcms::base_content {
	std::string title;
};

struct hello : public master {
	std::string msg;
};

};

