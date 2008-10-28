#ifndef DATA_H
#define DATA_H

#include <cppcms/base_view.h>
#include <cppcms/form.h>
#include <string>

class wiki;

namespace data {

using namespace std;
using namespace cppcms;

struct login_form : form {
	wiki *w;
	widgets::text username;
	widgets::password password;
	widgets::submit login;
	login_form(wiki *);
	virtual bool validate();
};

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

struct options_form : form {
	wiki *w;
	widgets::checkbox users_only;
	widgets::text wiki_title;
	widgets::textarea about;
	widgets::text copyright;
	widgets::submit submit;
	options_form(wiki *);
};

struct master : public cppcms::base_content {
	string media;
	string cookie_prefix;
	string main_link;
	string login_link;
	string toc;
	string edit_options;
	string wiki_title,about,copyright;
	map<string,string> languages;
	virtual string markdown(string);
};

struct edit_options:  public master {
	options_form form;
	edit_options(wiki *w) : form(w){}
};

struct login : public master {
	login_form form;
	login(wiki *w) : form(w){};
};

struct page : public master {
	string title,content;
	string sidebar;
	string edit_link;
	string history_link;
};

struct toc : public master {
	struct element {
		string letter;
		string title;
		string url;
	};
	typedef vector<element> column_t;
	column_t left_col,right_col,middle_col;
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
