#pragma once
#include <data/blog/master.h>

namespace data {
namespace blog {


	struct static_page : public master {
		int id;
		std::string title;
		std::string content;
		static_page() : id(0) {}
	};
}
}


