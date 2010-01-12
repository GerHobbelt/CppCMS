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

struct thread_shared : public master {
	string title;
	virtual string text2html(string const &s);
};

struct base_thread : public thread_shared {
	string flat_view;
	string tree_view;
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

typedef tree_thread::tree_msg::tree_t tree_t;

struct reply : public thread_shared , public msg {
	reply_form form;
	string back;
	reply(cppcms::application &a);
};

} // data


#endif
