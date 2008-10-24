#ifndef DATA_H
#define DATA_H

#include <cppcms/base_view.h>
#include <string>

class wiki;

namespace data {

using namespace std;
using namespace cppcms;

struct page_form : form {
	widgets::text title;
	widgets::textarea content;
	widgets::submit save;
	widgets::submit save_cont;
	widgets::submit preview;
	widgetset fields;
	widgetset buttons;
	page_form(wiki *w);
};

struct master : public cppcms::base_content {
	string media;
	virtual string markdown(string);
};

struct page : public master {
	string title,content;
}

struct edit_page: public page {
	page_form form;
	bool new_page;
	edit_page(wiki *w) : form(w),new_page(false) {}
};

} // data

#endif
