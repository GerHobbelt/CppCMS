#pragma once

#include <data/blog/post.h>

namespace data {
namespace blog {
	struct summary : public master {
		std::string category_name;
		int id;
		int prev_page;
		int page;
		int next_page;
		typedef std::vector<post_content> posts_type;
		posts_type posts;

		summary() : 
			id(0) ,
			prev_page(0),
			next_page(0)
		{
		}
	};

}
}
