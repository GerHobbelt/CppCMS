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

#include <string>
#include <ctime>

struct user_t {
	int 		id;
	std::string	username;
	std::string	password;
};

struct post_t {
	int		id;
	int		author_id;
	std::string	title;
	std::string	abstract;
	std::string	content;
	std::tm 	publish;
	int 		is_open;
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

typedef enum { BLOG_TITLE, BLOG_DESCRIPTION } case_t;

struct option_t {
	int		id;
	std::string	value;
};

#endif
