#pragma once
#include <data/admin/master.h>

namespace data {
	namespace admin {
		struct summary : public master {
			struct post {
				int id;
				std::string title;
				post() : id(0) {}
			};
			struct page {
				int id;
				std::string title;
				int is_open;
				page() : id(0), is_open(0) {}
			};
			struct comment {
				int id;
				int post_id;
				std::string username;
				comment() : id(0), post_id(0) {}
			};
			std::vector<comment> comments;
			std::vector<page> pages;
			std::vector<post> posts;
		};
	}
}
