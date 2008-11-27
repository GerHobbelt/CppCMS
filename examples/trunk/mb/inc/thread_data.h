#ifndef THREAD_DATA_H
#define THREAD_DATA_H
#include "master_data.h"
#include <cppcms/form.h>

namespace data {
using namespace cppcms;
struct reply_form : public form {
	widgets::text author;
	widgets::textarea comment;
	widgets::submit send;
	reply_form(cppcms::application &a);
};


struct msg {
	string author;
	string content;
	string reply_url;
};

struct base_thread : public master {
	string title;
	string reply_to_thread;
	virtual string text2html(string const &s);
};

struct flat_thread : public base_thread {
	vector<msg> messages;
};

struct tree_thread : public base_thread  {
	struct tree_msg : public msg {
		typedef map<int,tree_msg> tree_t;
		tree_t repl;
	};
	tree_msg::tree_t messages;
};

struct reply : public base_thread , public msg {
	reply_form form;
	reply(cppcms::application &a);
};

} // data


#endif
