#ifndef INDEX_DATA_H
#define INDEX_DATA_H

#include "master_data.h"

namespace data {

struct toc : public master {
	struct element {
		string letter;
		string title;
		string url;
	};
	typedef vector<element> column_t;
	column_t left_col,right_col,middle_col;
};

struct recent_changes : public master {
	struct element {
		string title;
		int version;
		std::tm created;
		string author;
		string url;
		string diff_url;
	};
	vector<element> data;
	string next;
};


} // namespace data


#endif
