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
#include <cppcms/archive.h>
#include <cppcms/base_view.h>
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
};
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

struct admin_editcomment : public admin_base  {
	int id;
	string edit_comment_url,author,email,url,content;
};
struct category {
	int id;
	string name;
	bool del;
};

struct admin_editcats : public admin_base {
	bool constraint_error;
	string submit_url;
	list<category> cats;

};

struct link {
	string name,descr,url;
	int id;
	int cat_id;
};

struct admin_editlinks : public admin_base {
	bool error_not_empty;
	string submit_url;
	list<category> link_cats;
	list<link> links;
};

struct admin_editoptions : public admin_base {
	string post_options_url,blog_description,blog_contact,copyright_string;
};

struct admin_editcontent : public admin_base {
	bool is_open;
	int post_id;
	string submit_post_url,post_title,content,preview_url;
};

struct admin_editpost : public admin_editcontent {
	list<category> cats_in,cats_out;
	string abstract,send_trackback_url;

};

struct admin_editpage : public admin_editcontent {};
struct admin_login : public admin_base {
	string login_url;
};

struct short_post {
	string title;
	string edit_url;
	bool published;
};

struct short_comment {
	string post_permlink,username,edit_url;
	int id;
};

struct admin_main : public admin_base {
	list<short_comment> comments;
	list<short_post> posts;
	list<short_post> pages;
};

struct admin_sendtrackback : public admin_base {
	bool error_no_url;
	string error_message;
	bool success;
	string goback;
};

};


#endif
