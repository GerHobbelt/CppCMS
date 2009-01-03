#include "wiki.h"
#include <boost/bind.hpp>
#include <cgicc/HTTPRedirectHeader.h>
#include <boost/format.hpp>
#include "utf8/utf8.h"

using namespace dbixx;
using cgicc::HTTPRedirectHeader;
using cgicc::HTTPCookie;
namespace apps {

wiki::wiki(worker_thread &w) :
	application(w),
	page(*this),
	options(*this),
	users(*this),
	index(*this)
{
	dbixx_load(sql);
	
	script=app.config.sval("wikipp.script");

	url.add("^/(\\w+)(/.*)$",
		boost::bind(&wiki::run,this,$1,$2));
		
	use_template("view");

}

string wiki::root(string l)
{
	if(l.empty()) l=locale;
	return script+"/"+l;
}

bool wiki::set_locale(string lang)
{
	if(lang!="en") {
		vector<string> const &lst=app.config.slist("locale.lang_list");
		vector<string>::const_iterator p,e;
		for(p=lst.begin(),e=lst.end();p!=e;++p) {
			if(lang==*p) {
				break;
			}
		}
		if(p==e) {
			return false;
		}
	}
	locale=lang;
	set_lang(lang);
	return true;
}

void wiki::on_404()
{
	page.redirect();
}

void wiki::run(string l,string u)
{
	if(!set_locale(l)) {
		on_404();
		return;
	}
	if(url_next.parse(u)<0) {
		page.redirect(l);
	}

}

}
