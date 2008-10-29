#ifndef WIKI_H
#define WIKI_H
#include <cppcms/worker_thread.h>
#include <cppcms/manager.h>
#include <dbixx/dbixx.h>
#include <boost/format.hpp>
#include <cppcms/archive.h>

#include "data.h"
#include "page.h"
#include "users.h"
#include "index.h"
#include "options.h"


using namespace cppcms;

class wiki;

class wiki : public worker_thread {
	friend class apps::page;
	friend class apps::options;
	friend class apps::users;
	friend class apps::index;
	friend class apps::master;

	// Internal Data
	typedef list<boost::function<void()> > on_load_lst_t;
	on_load_lst_t on_load_lst;
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

	// funtions
	void on_load(boost::function<void()>);

	string root(string locale="");
	virtual void main(); 
	bool set_locale(string);
	void run(string lang,string url);
	void set_cookies(string,string,int);
	wiki(manager const &s);
};


#endif

