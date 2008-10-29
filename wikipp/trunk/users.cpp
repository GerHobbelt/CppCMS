#include <cgicc/HTTPRedirectHeader.h>
#include "users.h"
#include "wiki.h"

using cgicc::HTTPRedirectHeader;
using cgicc::HTTPCookie;
using namespace dbixx;

namespace apps {

users::users(wiki &w) :	master(w)
{
	wi.url_next.add("^/login/?$",
		boost::bind(&users::login,this));
	wi.on_load(boost::bind(&users::reset,this));
	reset();
}

void users::reset()
{
	auth_done=auth_ok=false;
}

string users::login_url()
{
	return wi.root()+"/login/";
}

void users::login()
{
	data::login c(&wi);
	int time=3600*24*7;
	if(wi.env->getRequestMethod()=="POST") {
		c.form.load(*wi.cgi);
		if(c.form.validate()) {
			wi.page.redirect(locale);
			wi.set_cookies(c.form.username.get(),c.form.password.get(),time);
			return;
		}
	}
	else {
		if(auth()) {
			wi.set_header(new HTTPRedirectHeader(wi.env->getReferrer()));
			wi.set_cookies("","",-1);
			return;
		}
	}
	ini(c);
	wi.render("login",c);
}

bool users::check_login(string u,string p)
{
	if(u.empty() || p.empty())
		return false;
	wi.sql<<"SELECT password FROM users "
		"WHERE username=?",u;
	row r;
	if(!wi.sql.single(r) ) {
		return false;
	}
	string pass;
	r>>pass;
	if(p!=pass)
		return false;
	return true;
}

bool users::auth()
{
	if(!auth_done)
		do_auth();
	return auth_ok;
}

void users::do_auth()
{
	string tmp_username;
	string tmp_password;

	string cookie=wi.app.config.sval("wikipp.cookie_id","");
	const vector<HTTPCookie> &cookies = wi.env->getCookieList();
	unsigned i;
	for(i=0;i!=cookies.size();i++) {
		if(cookies[i].getName()==cookie + "username") {
			tmp_username=cookies[i].getValue();
		}
		else if(cookies[i].getName()==cookie + "password") {
			tmp_password=cookies[i].getValue();
		}
	}
	auth_ok=check_login(tmp_username,tmp_password);
}

void users::error_forbidden()
{
	wi.set_header(new HTTPRedirectHeader(login_url()));
}


}
