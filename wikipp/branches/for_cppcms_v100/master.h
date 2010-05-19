#ifndef MASTER_H
#define MASTER_H

#include <cppcms/application.h>
#include <map>

namespace content { class master; }
namespace dbixx { class session; }
namespace apps {

class wiki;

class master : public cppcms::application {
protected:
	wiki &wi;
	dbixx::session &sql;
	std::string &locale_name;
	std::map<std::string,std::string> languages;
public:
	master(wiki &w);
	void ini(content::master &);
private:
	std::string media;
	std::string syntax_highlighter;
	std::string cookie_prefix;
};

} // namespace wiki


#endif


