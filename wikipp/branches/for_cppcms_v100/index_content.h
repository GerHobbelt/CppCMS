#ifndef INDEX_CONTENT_H
#define INDEX_CONTENT_H

#include "master_content.h"

namespace content {

struct toc : public master {
	struct element {
		std::string letter;
		std::string title;
		std::string url;
	};
	typedef std::vector<element> column_t;
	column_t left_col,right_col,middle_col;
};

struct recent_changes : public master {
	struct element {
		std::string title;
		int version;
		time_t created;
		std::string author;
		std::string url;
		std::string diff_url;
	};
	std::vector<element> content;
	std::string next;
};


} // namespace content


#endif
