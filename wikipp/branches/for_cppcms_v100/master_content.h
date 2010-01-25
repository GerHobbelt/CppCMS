#ifndef MASTER_DATA_H
#define MASTER_DATA_H
#include <cppcms/view.h>
#include <cppcms/form.h>
#include <string>

namespace apps { class wiki; }

namespace content {
using namespace std;
using namespace cppcms;
using apps::wiki;


struct master : public cppcms::base_content {
	string media;
	string syntax_highlighter;
	string cookie_prefix;
	string main_link;
	string main_local;
	string login_link;
	string toc;
	string changes;
	string edit_options;
	string contact;
	string wiki_title,about,copyright;
	map<string,string> languages;
	virtual string markdown(string);
};

}


#endif
