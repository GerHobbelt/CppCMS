//
// C++ Interface: data
//
// Description:
//
//
// Author: artik <artik@artyom-linux>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef DATA_H
#define DATA_H
#include <boost/function.hpp>
#include <cppcms/archive.h>
#include <cppcms/base_view.h>
#include <cppcms/form.h>
#include <string>
#include <ctime>

using namespace cppcms;

struct user_t : public serializable {
	int 		id;
	std::string	username;
	std::string	password;
	virtual void load(archive &a) { a>>id>>username>>password; };
	virtual void save(archive &a) const { a<<id<<username<<password; };
};

struct post_t {
	int		id;
	int		author_id;
	std::string	title;
	std::string	abstract;
	std::string	content;
	std::tm 	publish;
	int 		is_open;
	int		comment_count;
	// Joined data
	std::string	author_name;
	int		has_content;
};

struct comment_t {
	int		id;
	int		post_id;
	std::string	author;
	std::string	email;
	std::string	url;
	std::tm		publish_time;
	std::string	content;
};

typedef enum { BLOG_TITLE, BLOG_DESCRIPTION, BLOG_CONTACT } case_t;

struct option_t {
	int		id;
	std::string	value;
};


namespace data {


struct common_base : public base_content {
	string media;
	string base_url;
	string blog_name;
	string cookie_prefix;
};

/* PAGES */

struct master : public common_base {

	boost::function<void(string &)> on_sidebar_load;
	boost::function<string(string)> markdown2html;
	boost::function<string(string)> latex;

	string sidebar;

	string subtitle,blog_description, rss_posts,rss_comments,category_rss,category_name;
	string admin_url,blog_contact;
	string host;
};

struct error : public master {
	bool error_404;
	bool error_comment;
	error() : error_404(false), error_comment(false) {}
};

struct category {
	int id;
	string name;
	string url;
	bool del;
};

struct comment_data {
	int id;
	std::tm date;
	string url,username,delete_url,content,edit_url;
};


struct post_data {
	int id;
	int is_open;
	string permlink,title,author,abstract;
	std::tm date;
	list<category> post_cats;
	int comment_count;
	string edit_url;
	int has_content;
};

struct post : public master,public post_data {
	string preview_message_content;
	string content;
	vector<comment_data> comments;
	string trackback_part_1,trackback_part_2;
	string post_comment_url_1,post_comment_url_2;
};

struct main_page : public master {
	vector<post_data> posts;
	string next_page_link;
};

struct page: public master {
	string title;
	string content;
};



struct comment_feed {
	int id;
	string author,content,permlink;
};

struct feed_comments : public master {
	vector<comment_feed> comments;
};


struct trackback: public base_content {
	int error;
	string message;
};

struct sidebar : public base_content {
	boost::function<string(string)> markdown2html;
	string copyright_string;
	struct page {string link,title;};
	vector<page> pages;
	struct cat { string link,name; };
	vector<cat> cats;
	struct link_cat {
		string title;
		struct link {
			string href,title,description;
		};
		list<link> links;
	};
	list<link_cat> link_cats;
};

/* ADMIN */

struct admin_base : public common_base  {
	string admin_url;
	string logout_url;
	string new_post_url;
	string new_page_url,edit_links_url,edit_cats_url,edit_options_url,admin_cache_url;
};

struct admin_cache : public admin_base {
	bool cache_enabled;
	int cache_keys;
	string submit_url;
};

struct edit_comment_form : public form {
	widgets::text author;
	widgets::email email;
	widgets::text  url;
	widgets::textarea content;
	widgets::submit save;
	widgets::submit del;
	edit_comment_form(worker_thread *w);
};

struct admin_editcomment : public admin_base  {
	int id;
	edit_comment_form form;
	admin_editcomment(worker_thread *w) : form(w){};
};
struct admin_editcats : public admin_base {
	bool constraint_error;
	string submit_url;
	vector<category> cats;

};

struct link {
	string name,descr,url;
	int id;
	int cat_id;
};

struct admin_editlinks : public admin_base {
	bool error_not_empty;
	string submit_url;
	vector<category> link_cats;
	vector<link> links;
};

struct admin_editoptions : public admin_base {
	string post_options_url,blog_description,blog_contact,copyright_string;
};

struct admin_editcontent : public admin_base {
	int is_open;
	int post_id;
	string submit_post_url,post_title,content,preview_url;
};

struct admin_editpost : public admin_editcontent {
	vector<category> cats_in;
	list<category> cats_out;
	string abstract,send_trackback_url;

};

struct admin_editpage : public admin_editcontent {};
struct admin_login : public admin_base {
	string login_url;
};

struct short_post {
	string title;
	string edit_url;
	int published;
};

struct short_comment {
	string post_permlink,username,edit_url;
	int id;
};

struct admin_main : public admin_base {
	vector<short_comment> comments;
	vector<short_post> posts;
	vector<short_post> pages;
};

struct admin_sendtrackback : public admin_base {
	bool error_no_url;
	string error_message;
	bool success;
	string goback;
};

struct admin_setupblog : public admin_base {
	bool password_error,configured,field_error;
	string submit;
};

};


#endif
