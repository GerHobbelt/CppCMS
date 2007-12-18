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

#include <cppcms/easy_bdb.h>

using namespace ebdb;

typedef varchar<32> username_t;
typedef varchar<64> name_t;
typedef varchar<64> url_t;
typedef varchar<32> email_t;
typedef varchar<16> password_t;

typedef varchar<128> title_t;
typedef varchar<64> slug_t;

typedef varchar<128> short_text_t;

struct user_t {
	int id;
	username_t	username;
	password_t	password;
};

class Users {
	public:
		typedef Index_Auto_Increment<user_t,int,&user_t::id> id_idx;
		typedef id_idx::cursor_t id_c;
		id_idx id;

		typedef Index_Var<user_t,username_t,&user_t::username> username_idx;
		typedef username_idx::cursor_t username_c;
		username_idx username;
	Users(Environment &env) :
			id(env,"users_id.db",DB_BTREE),
	username(env,"users_username.db",DB_BTREE,&id)
	{};
	void open() { id.open(); username.open();};
	void create() { id.create(); username.create();};
	void close() { username.close();id.close(); };
};


struct post_t {
	int id;
	int author_id;
	title_t title;
	long abstract_id;
	long content_id;
	time_t publish;
	bool is_open;
};


class Posts {
public:
	typedef Index_Auto_Increment<post_t,int,&post_t::id> id_t;
	typedef id_t::cursor_t id_c;
	id_t id;

	typedef Index_Var<post_t,time_t,&post_t::publish> publish_t;
	typedef publish_t::cursor_t publish_c;
	publish_t publish;
	Posts(Environment &env) :
		id(env,"posts_id.db",DB_BTREE),
		publish(env,"posts_publish.db",DB_BTREE,&id)
	{};
	void open() { id.open(); publish.open();};
	void create() { id.create(); publish.create();};
	void close() { publish.close(); id.close(); };
};

struct approved_t {
	name_t author;
	email_t email;
	typedef Ord_Pair<name_t,email_t> def_t;
	def_t get() { return def_t(author,email); };
};

class Approved : public Index_Func<approved_t,approved_t::def_t,&approved_t::get>
{
public:
	Approved(Environment &env) :
		Index_Func<approved_t,approved_t::def_t,&approved_t::get>(env,"approved.db",DB_BTREE)
	{};

};


struct comment_t {
	int id;
	int post_id;
	int author_id;
	name_t author;
	email_t email;
	url_t url;
	time_t publish_time;
	bool moderated;
	long content_id;
	typedef Ord_Pair<int,time_t> sec_t;
	sec_t secondary() { return sec_t(post_id,publish_time); };
};

class Comments {
public:
	typedef Index_Auto_Increment<comment_t,int,&comment_t::id> id_t;
	typedef id_t::cursor_t id_c;
	id_t id;

	typedef Index_Func<comment_t,comment_t::sec_t,&comment_t::secondary> posttime_t;
	typedef posttime_t::cursor_t posttime_c;
	posttime_t posttime;

	Comments(Environment &env) :
			id(env,"comments_id.db",DB_BTREE),
			posttime(env,"comments_posttime.db",DB_BTREE,&id,NOT_UNIQUE)
	{};
	void open() { id.open(); posttime.open();};
	void create() { id.create(); posttime.create();};
	void close() {  posttime.close();id.close();};
};

typedef enum { BLOG_TITLE, BLOG_DESCRIPTION } case_t;

struct option_t {
	case_t id;
	short_text_t value;
};


class Options : public Index_Var<option_t, case_t , &option_t::id>
{
public:
	Options(Environment &env) :
		Index_Var<option_t, case_t , &option_t::id>(env,"options.db",DB_BTREE)
	{};
};



extern std::auto_ptr<Options> options;
extern std::auto_ptr<Comments> comments;
extern std::auto_ptr<Approved> approved;
extern std::auto_ptr<Posts> posts;
extern std::auto_ptr<Users> users;
extern std::auto_ptr<Texts_Collector> texts;

void db_openall();
void db_createall();
void db_closeall();
void db_initall();
int  db_configure();

#endif
