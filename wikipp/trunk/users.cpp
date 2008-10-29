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
	disable_reg=app.config.lval("wikipp.disable_registration",1);
	if(!disable_reg){
		wi.url_next.add("^/register/?$",
			boost::bind(&users::new_user,this));
	}
	on_start.connect(boost::bind(&users::reset,this));
	reset();
}

void users::new_user()
{
	data::new_user c(&wi);
	if(env->getRequestMethod()=="POST") {
		c.form.load(*cgi);
		transaction tr(sql);
		if(c.form.validate()) {
			sql<<	"INSERT INTO users(username,password) "
				"VALUES(?,?)",
				c.form.username.get(),
				c.form.password1.get(),
				exec();
			tr.commit();
			wi.page.redirect(locale);
			wi.set_cookies(c.form.username.get(),c.form.password1.get(),3600*7*24);
			return;
		}
		tr.commit();
	}
	ini(c);
	render("new_user",c);
}

void users::reset()
{
	auth_done=auth_ok=false;
}

string users::login_url()
{
	return wi.root()+"/login/";
}

bool users::user_exists(string u)
{
	sql<<"SELECT id FROM users WHERE username=?",u;
	row r;
	return sql.single(r);
}

void users::login()
{
	data::login c(&wi);
	int time=3600*24*7;
	if(env->getRequestMethod()=="POST") {
		c.form.load(*cgi);
		if(c.form.validate()) {
			wi.page.redirect(locale);
			wi.set_cookies(c.form.username.get(),c.form.password.get(),time);
			return;
		}
	}
	else {
		if(auth()) {
			set_header(new HTTPRedirectHeader(env->getReferrer()));
			wi.set_cookies("","",-1);
			return;
		}
	}
	ini(c);
	if(!disable_reg)
		c.new_user=wi.root()+"/register/";
	render("login",c);
}

bool users::check_login(string u,string p)
{
	if(u.empty() || p.empty())
		return false;
	sql<<	"SELECT password FROM users "
		"WHERE username=?",u;
	row r;
	if(!sql.single(r) ) {
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

	string cookie=app.config.sval("wikipp.cookie_id","");
	const vector<HTTPCookie> &cookies = env->getCookieList();
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
	set_header(new HTTPRedirectHeader(login_url()));
}


}
