#include <cgicc/HTTPRedirectHeader.h>
#include "users.h"
#include "wiki.h"
#include "users_content.h"
#include <sys/time.h>
#include <time.h>

using cgicc::HTTPRedirectHeader;
using namespace dbixx;

namespace content {
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
	captcha("capt",w->gettext("Solve")),
	submit("submit",w->gettext("Submit"))
{
	*this & username & password1 & password2 & captcha & submit;
	username.set_nonempty();
	password1.set_nonempty();
	password2.set_equal(password1);
}

void new_user_form::generate_captcha()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	unsigned seed=tv.tv_usec / 1000 % 100;
	int num1=rand_r(&seed) % 10+1;
	int num2=rand_r(&seed) % 10+1;
	int sol=num1+num2;
	captcha.help=(boost::format("%1% + %2%") % num1 % num2).str();
	w->session.set("captcha",sol);
	w->session.set_age(5*60); // at most 5 minutes
}

bool new_user_form::validate()
{
	if(!form::validate())
		return false;
	
	if(!w->session.is_set("captcha") || captcha.get()!=w->session["captcha"]) {
		w->session.del("captcha");
		return false;
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
	content::new_user c(&wi);
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
			session["username"]=c.form.username.get();
			session.expose("username");
			session.set_age(); // return to default
			return;
		}
		tr.commit();
	}
	c.form.generate_captcha();
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
	string key="user_exists_"+u;
	string tmp;
	if(cache.fetch_frame(key,tmp,true)) { // No triggers
		return true;
	}
	sql<<"SELECT id FROM users WHERE username=?",u;
	row r;
	if(sql.single(r)) {
		cache.store_frame(key,tmp);
		return true;
	}
	return false;
}

void users::login()
{
	content::login c(&wi);
	if(env->getRequestMethod()=="POST") {
		c.form.load(*cgi);
		if(c.form.validate()) {
			wi.page.redirect(locale);
			session["username"]=c.form.username.get();
			session.expose("username");
			return;
		}
	}
	else {
		if(auth()) {
			set_header(new HTTPRedirectHeader(env->getReferrer()));
			add_header("Status: 302 Found");
			session.clear();
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
	if(session.is_set("username") && user_exists(session["username"])) {
		auth_ok=true;
	}
	else {
		auth_ok=false;
	}
	if(auth_ok)
		username=session["username"];
	else
		username=env->getRemoteAddr();
	auth_done=true;
}

void users::error_forbidden()
{
	set_header(new HTTPRedirectHeader(login_url()));
	add_header("Status: 302 Found");
}


}
