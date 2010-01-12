#ifndef MASTER_DATA_H
#define MASTER_DATA_H

#include <cppcms/base_view.h>
using namespace cppcms;
namespace data {

struct master : public cppcms::base_content {
	std::string main_page;
	std::string media;
};


}

#endif

