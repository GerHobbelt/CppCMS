#ifndef WIKI_H
#define WIKI_H

#include "data.h"
#include <cppcms/worker_thread.h>
#include <dbixx/dbixx.h>

using namespace cppcms;

struct links_str {
	string main_page;
	string page;
	string edit_page;
};

class wiki : public worker_thread {
	dbixx::session sql;
	links_str links;
	url_parser url2;
	string locale;
public:
	virtual void main(); 
	void lang(string lang,string url);
	void page(string lang,string slug);
	void edit_page(string lang,string slug);
	wiki(manager const &s);
	virtual void main();
};


#endif

