#ifndef WIKI_H
#define WIKI_H
#include <cppcms/worker_thread.h>
#include <cppcms/manager.h>
#include <dbixx/dbixx.h>
#include <boost/format.hpp>
#include <cppcms/archive.h>

#include "page.h"
#include "users.h"
#include "index.h"
#include "options.h"


using namespace cppcms;

namespace apps {

class wiki : public application {
	friend class apps::page;
	friend class apps::options;
	friend class apps::users;
	friend class apps::index;
	friend class apps::master;

	string script;
public:
	// Data 
	dbixx::session sql;
	url_parser url_next;
	string locale;

	// Applications 

	apps::page page;
	apps::options options;
	apps::users users;
	apps::index index;

	string root(string locale="");
	bool set_locale(string);
	void run(string lang,string url);
	void set_cookies(string,string,int);
	virtual void on_404();
	wiki(worker_thread &w);
};

}

#endif

