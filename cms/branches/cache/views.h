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
#include <tmpl/content.h>

using boost::shared_ptr;
using namespace tmpl;
using namespace std;
class Blog;

class View_Comment {
	Blog *blog;
	content &c;
public:
	View_Comment(Blog *b,content &con): c(con) { blog=b; };
	void init(comment_t &c);
};

class View_Post {
	friend class View_Main_Page;
	Blog *blog;

	void ini_share(post_t &p);
	content &c;

public:
	View_Post(Blog *b,content &con): c(con) { blog=b; };
	void ini_short(post_t &p);
	void ini_full(post_t &p);
};


class View_Main_Page {
	Blog *blog;
	int error_code;
	void ini_share();
	void ini_sidebar();
	void prepare_query(int,int,int);
	content &c;
public:
	View_Main_Page(Blog *blog,content &con) : c(con)
	{
		this->blog=blog;
	};
	void ini_post(int id,bool preview);
	void ini_page(int id,bool preview);
	void ini_main(int id=-1,bool feed=false,int cat_id=-1);
	void ini_error(int what);
	void ini_rss_comments();
};

class View_Admin_Post {
	Blog *blog;
	content &c;
public:
	View_Admin_Post(Blog *b,content &con) : c(con) { blog=b;};
	void ini(int id,string ptype);
};

class View_Admin_Main
{
	Blog *blog;
	content &c;

public:
	View_Admin_Main(Blog *b,content &con): blog(b),c(con){};
	void ini();
};

class View_Admin {
	Blog *blog;
	content &c;
public:
	View_Admin(Blog *b,content &con) : blog(b),c(con) {};
	void ini_share();
	void ini_main();
	void ini_edit(int id,string ptype);
	void ini_options();
	void ini_links();
	void ini_cats();
	void ini_cedit(int id);
	void ini_login();
};

#endif
