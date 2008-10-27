#ifndef WIKI_H
#define WIKI_H

#include "data.h"
#include <cppcms/worker_thread.h>
#include <cppcms/manager.h>
#include <dbixx/dbixx.h>
#include <boost/format.hpp>

using namespace cppcms;

class wiki;
struct links_str {
	wiki *w;
	string main_page;
	string page;
	string edit_page;
	string edit_version;
	string rollback;
	string history;
	string history_next;
	string page_hist;
	string login;
	string admin;
	boost::format url(string);
	boost::format admin_url(string);
	links_str(wiki *);
};

class wiki : public worker_thread {
	friend class links_str;
	friend class login_form;
	dbixx::session sql;
	links_str links;
	url_parser url2;
	url_parser url_admin;
	string locale;
	string slug;
	bool auth_done;
	bool auth_ok;
	void do_auth();
public:
	virtual void main(); 
	bool auth();
	int default_edit_perssions();
	bool set_locale(string);
	void save_page(int id,data::page_form &);
	bool load_page(data::page_form &);
	bool on_edit_post(data::edit_page &);
	bool load_history_page(int,data::page_form &);
	void on_login();
	bool check_login(string,string);
	void set_cookies(string,string,int);
	void error_forbidden();
	void lang(string lang,string slug,string url);
	void admin(string lang,string url);
	void page();
	void edit_page(string version);
	void ini_share(data::page &c);
	void ini_master(data::master &c);
	void page_hist(string sid);
	void history(string page);
	void redirect(string loc="en",string page="main");
	wiki(manager const &s);
};


#endif

