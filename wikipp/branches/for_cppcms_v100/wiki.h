#ifndef WIKI_H
#define WIKI_H
#include <cppcms/application.h>
#include <dbixx/dbixx.h>
#include <boost/format.hpp>

#include "page.h"
#include "users.h"
#include "index.h"
#include "options.h"



namespace apps {

class wiki : public cppcms::application {
	friend class apps::page;
	friend class apps::options;
	friend class apps::users;
	friend class apps::index;
	friend class apps::master;

	std::string script;
public:
	// Data 
	dbixx::session sql;
	std::string locale_name;

	// Applications 

	apps::page page;
	apps::options options;
	apps::users users;
	apps::index index;

	std::string root(string locale_name="");
	bool set_locale(string);
	void run(string lang,string url);
	virtual void on_404();
	virtual void main(std::string url);
	wiki(cppcms::service &s);
};

}

#endif

