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

class View_Comment {
	Blog *blog;
	string author;
	string url;
	string message;
	string date;
public:
	View_Comment(Blog *b) { blog=b; };
	void ini(comment_t &c);
	int render(Renderer &r,Content &c, string &out);
};

class View_Post {
	Blog *blog;
	int id;
	string title;
	string permlink;
	string author;
	string abstract;
	string content;
	string date;
	bool has_content;
	string post_comment;
	bool has_comments;
	list<View_Comment> comments;
	void ini_share(post_t &p);
public:
	View_Post(Blog *b) { blog=b; };
	void ini_feed(post_6 &p);
	void ini_short(post_t &p);
	void ini_full(post_t &p);
	int render(Renderer &r,Content &c, string &out);
};


class View_Main_Page {
	Blog *blog;
	string title;
	string description;
	shared_ptr<View_Post> single_post;
	vector<View_Post> latest_posts;
public:	
	View_Main_Page(Blog *blog)
	{
		this->blog=blog;
	};
	void ini_post();
	void ini_main(int id=-1);
	void ini_feed();
	int render(Renderer &r,Content &c, string &out);
	
};