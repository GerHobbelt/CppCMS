#include "wiki.h"
#include <boost/bind.hpp>
#include <cgicc/HTTPRedirectHeader.h>
#include <boost/format.hpp>
#include "utf8/utf8.h"

using namespace dbixx;
using cgicc::HTTPRedirectHeader;
using cgicc::HTTPCookie;

void wiki::set_cookies(string p,string u,int time)
{
	string cookie=app.config.sval("wikipp.cookie_id","");
	HTTPCookie u_c(cookie + "username",p,"","",time,"/",false);
	set_cookie(u_c);
	HTTPCookie p_c(cookie + "password",u,"","",time,"/",false);
	set_cookie(p_c);
}

void wiki::on_load(boost::function<void()> f)
{
	on_load_lst.push_back(f);
}

wiki::wiki(manager const &s) :
	worker_thread(s),
	page(*this),
	options(*this),
	users(*this),
	index(*this)
{
	string engine=app.config.sval("dbi.engine");
	sql.driver(engine);
	if(engine=="mysql"){
		sql.param("dbname",app.config.sval("mysql.db"));
		sql.param("username",app.config.sval("mysql.user"));
		sql.param("password",app.config.sval("mysql.pass"));
	}
	else if(engine=="sqlite3") {
		sql.param("dbname",app.config.sval("sqlite3.db"));
		sql.param("sqlite3_dbdir",app.config.sval("sqlite3.dir"));
	}

	sql.connect();
	
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


void wiki::main()
{
	for(on_load_lst_t::iterator p=on_load_lst.begin(),e=on_load_lst.end();p!=e;++p) {
		(*p)();
	}
	if(url.parse()<0) {
		page.redirect();
	}
}

void wiki::run(string l,string u)
{
	if(!set_locale(l)) {
		page.redirect();
		return;
	}
	if(url_next.parse(u)<0) {
		page.redirect(l);
	}

}

