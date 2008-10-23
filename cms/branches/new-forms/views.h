//
// C++ Implementation: views
//
// Description:
//
//
// Author: artik <artik@art-laptop>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef VIEWS_H
#define VIEWS_H

#include <boost/shared_ptr.hpp>
#include "data.h"
#include <list>
#include <vector>
#include <set>

using boost::shared_ptr;
using namespace std;
class Blog;


class View_Post {
	friend class View_Main_Page;
	Blog *blog;

	void ini_share(data::post_data &);

public:
	View_Post(Blog *b): blog(b) {}
	void ini_short(data::post_data &);
	void ini_full(data::post &);
};


class View_Main_Page {
	Blog *blog;
	int error_code;
	void ini_share(data::master &c);
	void ini_sidebar(set<string> &triggers,data::sidebar &c);
	void prepare_query(int,int,int);
	void on_sidebar_load(string &sidebar);
public:
	View_Main_Page(Blog *b) : blog(b) {}
	void ini_post(int id,bool preview,data::post &c);
	void ini_page(int id,bool preview,data::page &c);
	void ini_main(int id,bool feed,int cat_id,data::main_page &c);
	void ini_error(int what,data::error &c);
	void ini_rss_comments(data::feed_comments &c);
};

class View_Admin {
	Blog *blog;
public:
	View_Admin(Blog *b) : blog(b) {};
	void ini_share(data::admin_base &c);
	void ini_main(data::admin_main &c);
	void ini_editpage(int id,data::admin_editpage &c);
	void ini_editpost(int id,data::admin_editpost &c);
	void ini_options(data::admin_editoptions &c);
	void ini_links(data::admin_editlinks &c);
	void ini_cats(data::admin_editcats &c);
	void ini_login(data::admin_login &c);
};

#endif
