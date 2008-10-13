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
using namespace tmpl;
using namespace std;
class Blog;

class View_Comment {
	Blog *blog;
public:
	View_Comment(Blog *b) blog(b){}
	void init(comment_t &c);
};

class View_Post {
	friend class View_Main_Page;
	Blog *blog;

	void ini_share(post_t &p);

public:
	View_Post(Blog *b): blog(b) {}
	void ini_short(post_t &p);
	void ini_full(post_t &p);
};


class View_Main_Page {
	Blog *blog;
	int error_code;
	void ini_share(data::master &c);
	void ini_sidebar(set<string> &triggers,data::sidebar &c);
	void prepare_query(int,int,int);
public:
	View_Main_Page(Blog *b) : blog(b) {}
	void ini_post(int id,bool preview);
	void ini_page(int id,bool preview,data::page &c);
	void ini_main(int id=-1,bool feed=false,int cat_id=-1);
	void ini_error(int what);
	void ini_rss_comments();
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
	void ini_cedit(int id,data::admin_editcomment &c);
	void ini_login(data::admin_login &c);
};

#endif
