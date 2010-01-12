#ifndef DATA_H
#define DATA_H

#include <cppcms/view.h>
#include <string>

namespace content  {
	struct message : public cppcms::base_content {
		std::string message;
	};
}


#endif
