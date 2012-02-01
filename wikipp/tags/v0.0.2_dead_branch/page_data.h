#ifndef PAGE_DATA_H
#define PAGE_DATA_H

#include "master_data.h"

namespace data {

typedef list<std::pair<int,string> > diff_t;

struct page_form : public form {
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

struct edit_page: public page {
	page_form form;
	bool new_page;
	string back;
	string submit;
	edit_page(wiki *w) : form(w),new_page(false) {}
};

struct history : public master {
	struct item {
		std::tm update;
		string show_url;
		string edit_url;
		string diff_url;
		int version;
		string author;
	};
	vector<item> hist;
	string page;
	string title;
	string page_link;
};

struct diff: public master {
	string edit_v1,edit_v2;
	int v1,v2;
	diff_t content_diff_data;
	diff_t sidebar_diff_data;
	string title,title_1,title_2;
	bool title_diff,content_diff,sidebar_diff,no_versions,no_diff;
	diff() : 
		title_diff(false),content_diff(false),
		sidebar_diff(false),no_versions(false),
		no_diff(false)
	{
	}
};


} // namespace data

#endif
