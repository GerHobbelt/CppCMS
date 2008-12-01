#ifndef FORUMS_DATA_H
#define FORUMS_DATA_H
#include "master_data.h"
#include <cppcms/form.h>

namespace cppcms { class application; }

namespace data {

struct new_topic_form : public cppcms::form {
	widgets::text title;
	widgets::text author;
	widgets::textarea comment;
	widgets::submit submit;
	new_topic_form(cppcms::application &app);
};

struct forums : public master {
	struct topic {
		string title;
		string url;
	};
	vector<topic> topics;
	string next_page,prev_page;
	new_topic_form form;
	forums(cppcms::application &a) : form(a) {};
};

};


#endif
