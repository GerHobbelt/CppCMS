#pragma once
#include <cppcms/view.h>
#include <vector>
#include "data/basic_master.h"

namespace data {
	namespace feed {
		struct master : public data::basic_master {
			std::string blog_name;
			std::string blog_description;
			std::string host;
		};
		struct comment {
			int id;
			int post_id;
			std::string author;
			std::string content;
			comment() : id(0), post_id(0) {}
		};
		struct comments : public master {
			typedef std::vector<comment> items_type;
			items_type items;
		};
		struct post {
			int id;
			std::string title;
			std::string abstract;
			bool has_content;
			post() : id(0), has_content(false) {}
		};

		struct posts : public master {
			int id;
			std::string category;
			typedef std::vector<post> items_type;
			items_type items;
			posts() : id(0) {}
		};
	}
}
