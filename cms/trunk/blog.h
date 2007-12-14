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

#include <cppcms/worker_thread.h>

struct links_t {
	string media;
	string main;
	string main_from;
	string post;
	string admin;
	string new_post;
	string edit_post;
	string edit_comment;
	string add_comment;
	string add_post;
	string approve;
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

class Blog {
	Url_Parser url;
	void main_page(string s);
	links_t fmt;
// Member functions:
	void base_content(Content &c);
	void render_post(post_t const &p,bool include_content,Content &c);

public:
	virtual void init();
	virtual void main();
	Blog() : url(this) {};
};