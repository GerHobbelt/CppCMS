//
// C++ Interface: blog
//
// Description:
//
//
// Author: artik <artik@art-laptop>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef BLOG_H
#define BLOG_H
#include <cppcms/worker_thread.h>
#include <cppcms/url.h>
#include <dbi/dbixx.h>
#include <tmpl/renderer.h>
#include <tmpl/content.h>
#include <tmpl/transtext.h>
#include "views.h"
#include "data.h"


extern tmpl::template_data global_template;
extern transtext::trans_factory tr;
extern transtext::trans_gnu gnugt;

struct links_t {
	string media;
	string main;
	string main_from;
	string post;
	string page;
	string admin;
	string new_post;
	string new_page;
	string edit_post;
	string edit_page;
	string edit_comment;
	string add_comment;
	string add_post;
	string add_page;
	string update_post;
	string update_page;
	string approve;
	string login;
	string logout;
	string preview;
	string page_preview;
	string del_comment;
	string feed;
	string feed_comments;
	string update_comment;
	string lang_switch;
};

struct post_content_t {
	bool has_content;
	string title;
	string permlink;
	string author;
	string published;
	string abstract;
	string content;
};

class Blog : public Worker_Thread {
	URL_Parser url;
// Member functions:
	void main_page(string s);
	void post(string s,bool preview);
	void page(string s,bool preview);
	void add_comment(string &postid);
	void error_page(int);
	void setup_blog();

public:
	string username;
	int userid;
	bool connected;
	dbixx::session sql;
private:

	void auth_or_throw();

	void admin();
	void edit_post(string id,string ptype);
	void edit_comment(string id);
	void update_comment(string id);
	void get_post(string id,string type);
	void login();
	void logout();
	void save_post(int &id,string &title,
		       string &abstract,string &content,bool pub);
	void save_page(int &id,string &title,
		       string &content,bool pub);

	void set_login_cookies(string username,string password,int days);
	int check_login(string username,string password);
	void del_comment(string id);
	bool auth();
	void feed();
	void feed_comments();
	tmpl::renderer render;
	void set_lang();

	content c;

public:
	links_t fmt;
	virtual void init();
	virtual void main();
	void date(std::tm t,string &s);
	Blog() :
		url(this),
		render(global_template)
	{};
};
#endif
