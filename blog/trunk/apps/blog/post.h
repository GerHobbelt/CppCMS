#pragma once
#include <apps/blog/master.h>
namespace data {
	namespace blog {
		struct post;
	}
}

namespace apps {
namespace blog {

class post : public master {
public:
	post(cppcms::service &s);

	void display(std::string id);
	void preview(std::string id);
	void postback(std::string id);
private:
	bool prepare_content(data::blog::post &c,std::string const &id,bool preview);
	
};

} // blog
} // apps
