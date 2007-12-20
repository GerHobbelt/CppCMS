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

#include <cppcms/templates.h>
#include <boost/shared_ptr.hpp>
#include "data.h"
#include <list>
#include <vector>

using boost::shared_ptr;

class Blog;

class View_Comment {
	Blog *blog;
	string author;
	string url;
	string message;
	string date;
public:
	View_Comment(Blog *b) { blog=b; };
	void init(comment_t &c);
	int render(Renderer &r,Content &c, string &out);
};

class View_Post {
	friend class View_Main_Page;
	Blog *blog;
	int id;
	string title;
	string permlink;
	string author;
	string abstract;
	string content;
	string date;
	bool has_content;
	string post_comment1;
	string post_comment2;
	bool has_comments;
	list<shared_ptr<View_Comment> > comments_list;
	void ini_share(post_t &p);
public:
	View_Post(Blog *b) { blog=b; };
	void ini_feed(post_t &p);
	void ini_short(post_t &p);
	void ini_full(post_t &p);
	int render(Renderer &r,Content &c, string &out);
};


class View_Main_Page {
	Blog *blog;
	string title;
	string description;
	shared_ptr<View_Post> single_post;
	vector<shared_ptr<View_Post> > latest_posts;
	string from;
	enum { SUMMARY, SINGLE, ERROR } disp;
	int error_code;
	void ini_share();
public:
	View_Main_Page(Blog *blog)
	{
		this->blog=blog;
	};
	void ini_post(int id);
	void ini_main(int id=-1);
	void ini_error(int what);
	int render(Renderer &r,Content &c, string &out);
};


#endif
