#include "wiki.h"
#include <booster/regex.h>

#include <cppcms/url_dispatcher.h>
#include <cppcms/http_context.h>

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
	cppcms::json::object langs = settings().at("wikipp.languages").object();
	for(cppcms::json::object::const_iterator p=langs.begin();p!=langs.end();++p) {
		lang_map[p->first]=p->second.str();
	}
}

std::string wiki::root(std::string l)
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
		std::map<std::string,std::string>::const_iterator p = lang_map.find(std::string(res[1]));
		if(p==lang_map.end()) {
			page.redirect();
		}
		else {
			locale_name = p->first;
			context().locale(p->second);
			if(!dispatcher().dispatch(res[2]))
				page.redirect(locale_name);
		}
	}
	else
		page.redirect();
}


} // apps
