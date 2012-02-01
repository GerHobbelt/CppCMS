#pragma once
#include <data/blog/master.h>

namespace data {
namespace blog {


	struct page : public master {
		int id;
		std::string title;
		std::string content;
		page() : id(0) {}
	};
}
}


