#include "wiki.h"
#include <booster/regex.h>
#include "utf8/utf8.h"

#include <cppcms/url_dispatcher.h>

using namespace dbixx;
namespace apps {

wiki::wiki(cppcms::service &srv) :
	cppcms::application(srv),
	page(*this),
	options(*this),
	users(*this),
	index(*this)
{
	add(page);
	add(options);
	add(users);
	add(index);

	sql.driver(settings().get<std::string>("wikipp.sql.driver"));
	cppcms::json::object ob=settings().get<cppcms::json::object>("wikipp.sql.params");
	for(cppcms::json::object::const_iterator p=ob.begin();p!=ob.end();++p) {
		if(p->second.type()==cppcms::json::is_string)
			sql.param(p->first,p->second.str());
		else if(p->second.type()==cppcms::json::is_number)
			sql.param(p->first,p->second.number());
	}
	sql.connect();
	script=settings().get<std::string>("wikipp.script");
}

string wiki::root(string l)
{
	if(l.empty()) l=locale_name;
	return script+"/"+l;
}
static const booster::regex lang_regex("^/(\\w+)(/.*)?$");

void wiki::main(std::string url)
{
	booster::smatch res;
	options.reset();
	users.reset();
	if(booster::regex_match(url,res,lang_regex)) {
		std::string loc = settings().get("wikipp.languages." + res[1],"");
		if(loc.empty()) {
			page.redirect();
		}
		else {
			locale_name = res[1];
			locale(loc);
			if(!dispatcher().dispatch(res[2]))
				page.redirect(locale_name);
		}
	}
	else
		page.redirect();
}


} // apps
