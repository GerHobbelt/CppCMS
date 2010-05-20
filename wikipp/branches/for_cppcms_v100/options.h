#ifndef OPTIONS_H
#define OPTIONS_H
#include "master.h"
#include <cppcms/json.h>

namespace apps {

struct global_options {
	int users_only_edit;
	std::string contact;
};

struct locale_options {
	std::string title;
	std::string about;
	std::string copyright;
};




class options : public master {
	bool loaded;
protected:
	void edit();
public:
	void reset();
	options(apps::wiki &);
	void load();
	void save();
	std::string edit_url();
	global_options global;
	locale_options  local;
};

} // apps;
namespace cppcms { namespace json {

template<>					
struct traits<apps::global_options> {				
	static apps::global_options get(value const &v)		
	{					
		apps::global_options tmp;
		tmp.users_only_edit = v.get<bool>("users_only_edit");
		tmp.contact = v.get<std::string>("contact");
		return tmp;
	}					
	static void set(value &v,apps::global_options const &in)
	{					
		v = object();
		v["users_only_edit"]=bool(in.users_only_edit);
		v["contact"]=in.contact;
	}
};

template<>					
struct traits<apps::locale_options> {				
	static apps::locale_options get(value const &v)		
	{					
		apps::locale_options tmp;
		tmp.title = v.get<std::string>("title");
		tmp.copyright = v.get<std::string>("copyright");
		tmp.about = v.get<std::string>("about");
		return tmp;
	}					
	static void set(value &v,apps::locale_options const &in)
	{					
		v = object();
		v["title"]=in.title;
		v["about"]=in.about;
		v["copyright"]=in.copyright;
	}
};

}}//json::cppcms

#endif
