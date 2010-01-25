#include "wiki.h"
#include <boost/bind.hpp>
#include <cgicc/HTTPRedirectHeader.h>
#include <boost/format.hpp>
#include "utf8/utf8.h"

using namespace dbixx;
using cgicc::HTTPRedirectHeader;
namespace apps {

wiki::wiki(worker_thread &w) :
	application(w),
	page(*this),
	options(*this),
	users(*this),
	index(*this),
	lang_regex("^/(\\w+)(/.*)?$");
{
//	dbixx_load(sql);
	script=settings().get<std::string>("wikipp.script");
}

string wiki::root(string l)
{
	if(l.empty()) l=locale_name;
	return script+"/"+l;
}

void wiki::main(std::string url)
{
	cppcms::regex_result res;
	options.reset();
	if(lang_regex.match(url,res)) {
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
	page.redirect();
}


} // apps
