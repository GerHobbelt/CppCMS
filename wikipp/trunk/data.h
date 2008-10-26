#ifndef DATA_H
#define DATA_H

#include <cppcms/base_view.h>
#include <cppcms/form.h>
#include <string>

class wiki;

namespace data {

using namespace std;
using namespace cppcms;

struct page_form : form {
	wiki *w;
	widgets::text title;
	widgets::textarea content;
	widgets::textarea sidebar;
	widgets::submit save;
	widgets::submit save_cont;
	widgets::submit preview;
	widgets::checkbox users_only;
	widgetset fields;
	widgetset buttons;
	page_form(wiki *w);
	bool virtual validate();
};

struct master : public cppcms::base_content {
	string media;
	string main_link;
	map<string,string> languages;
	virtual string markdown(string);
};

struct page : public master {
	string title,content;
	string sidebar;
	string edit_link;
	string history_link;
};

struct page_hist: public page {
	int version;
	string rollback;
	std::tm date;
};

struct history : public master {
	struct item {
		std::tm update;
		string show_url;
		string edit_url;
		int version;
	};
	vector<item> hist;
	string page;
	string title;
	string page_link;
};

struct edit_page: public page {
	page_form form;
	bool new_page;
	string back;
	edit_page(wiki *w) : form(w),new_page(false) {}
};

} // data

#endif
