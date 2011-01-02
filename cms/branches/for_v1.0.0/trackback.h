#ifndef TRACKBACK_H
#define TRACKBACK_H
#include <string>


class trackback {
	std::string post_data;
	std::string target;
	std::string charset;
	void set_data(std::string name,std::string value);
public:
	trackback(std::string v,std::string cs) : target(v),charset(cs) {};
	void url(std::string v) { set_data("url",v); };
	void title(std::string v){ set_data("title",v); };
	void blog_name(std::string v){ set_data("blog_name",v); };
	void excerpt(std::string v){ set_data("excerpt",v); };
	bool post(std::string &error_message);
};

#endif
