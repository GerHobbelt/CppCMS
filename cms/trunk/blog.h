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
#include <set>
#include "views.h"
#include "data.h"

using namespace std;
using namespace cppcms;

extern tmpl::template_data global_template;
extern transtext::trans_factory tr;
extern transtext::trans_gnu gnugt;

struct links_t {
	string media;
	string cat;
	string cat_from;
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
	string edit_links;
	string edit_cats;
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
	string feed_cats;
	string feed_comments;
	string update_comment;
	string lang_switch;
	string edit_options;
	string trackback;
	string send_trackback;
	string admin_cache;
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

class Blog : public cppcms::worker_thread {
	friend class View_Main_Page;
	friend class View_Admin;
// Member functions:
	void main_page(string s,string);
	void post(string s,bool preview);
	void page(string s,bool preview);
	void add_comment(string &postid);
	void error_page(int);
	void setup_blog();
	void edit_links();

public:
	string username;
	int userid;
	bool connected;
	dbixx::session sql;
private:
	void create_gif(string const &tex,string const &fname);
	void latex_filter(string const &in,string &out);
	void auth_or_throw();

	void admin();
	void edit_post(string id,string ptype);
	void edit_comment(string id);
	void update_comment(string id);
	void get_post(string id,string type);
	void login();
	void logout();
	void save_post(int &id,string &title,
		       string &abstract,string &content,int pub,
		       set<int> &,set<int>&);
	void save_page(int &id,string &title,
		       string &content,int pub);

	void set_login_cookies(string username,string password,int days);
	int check_login(string username,string password);
	void del_comment(string id);
	void del_single_comment(int id);
	bool auth();
	void feed(string scatid);
	void feed_comments();
	tmpl::renderer render;
	void set_lang();
	void edit_options();
	void edit_cats();
	void count_comments(int id);
	void trackback(string sid);
	void send_trackback();
	void admin_cache();

	content c;
	void init();

public:
	links_t fmt;
	virtual void main();
	void date(std::tm t,string &s);
	Blog(manager const &s) :
		worker_thread(s),
		render(global_template)
	{
		init();
	};
};
#endif
