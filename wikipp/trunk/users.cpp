#include <cgicc/HTTPRedirectHeader.h>
#include "users.h"
#include "wiki.h"
#include "users_data.h"

using cgicc::HTTPRedirectHeader;
using cgicc::HTTPCookie;
using namespace dbixx;

namespace data {
login_form::login_form(wiki *_w) :
	w(_w),
	username("username",w->gettext("Username")),
	password("password",w->gettext("Password")),
	login("login",w->gettext("Login"))
{
	*this & username & password & login;
	username.set_nonempty();
	password.set_nonempty();
}

bool login_form::validate()
{
	if(!form::validate())
		return false;
	if(w->users.check_login(username.get(),password.get()))
		return true;
	password.not_valid();
	return false;
}



new_user_form::new_user_form(wiki *_w):
	w(_w),
	username("username",w->gettext("Username")),
	password1("p1",w->gettext("Password")),
	password2("p2",w->gettext("Confirm")),
	submit("submit",w->gettext("Submit"))
{
	*this & username & password1 & password2 ;
	username.set_nonempty();
	password1.set_nonempty();
	password2.set_equal(password1);

	vector<string> const &quiz=w->app.config.slist("wikipp.quiz_q");
	int i=1;
	for(vector<string>::const_iterator p=quiz.begin(),e=quiz.end();p!=e;++p) {
		this->quiz.push_back(widgets::checkbox((boost::format("%d") % i).str()));
		this->quiz.back().help=*p; 
		*this & this->quiz.back();
		i++;
	}
	*this & submit;
}

bool new_user_form::validate()
{
	if(!form::validate())
		return false;
	vector<int> const &quiz=w->app.config.llist("wikipp.quiz_a");
	list<widgets::checkbox>::iterator qp=this->quiz.begin();
	for(vector<int>::const_iterator p=quiz.begin(),e=quiz.end();p!=e;++p) {
		if(qp==this->quiz.end() || qp->get()!=*p)
			return false;
		++qp;
	}
	if(w->users.user_exists(username.get())) {
		username.error_msg=w->gettext("This user exists");
		username.not_valid();
		return false;
	}
	return true;
}


} 

namespace apps {

users::users(wiki &w) :	master(w)
{
	wi.url_next.add("^/login/?$",
		boost::bind(&users::login,this));
	disable_reg=app.config.get<int>("wikipp.disable_registration",1);
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
			add_header("Status: 302 Found");
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
	if(auth_ok)
		username=tmp_username;
	else
		username=env->getRemoteAddr();
}

void users::error_forbidden()
{
	set_header(new HTTPRedirectHeader(login_url()));
	add_header("Status: 302 Found");
}


}
