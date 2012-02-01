#pragma once
#include <apps/blog/master.h>

namespace apps {
namespace blog {

	class page : public master {
	public:
		page(cppcms::service &s);

		void display(std::string id);
		void preview(std::string id);
	private:
		bool prepare(std::string const &id,bool preview);
	};

} // blog
} // apps
