#ifndef DATA_H
#define DATA_H

#include <cppcms/base_view.h>
#include <string>

namespace data  {
	struct message : public cppcms::base_content {
		std::string message;
	};
}


#endif
