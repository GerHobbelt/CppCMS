#include "wiki.h"
#include <boost/bind.hpp>
#include <cgicc/HTTPRedirectHeader.h>
#include <boost/format.hpp>
#include "utf8/utf8.h"

using namespace dbixx;
using cgicc::HTTPRedirectHeader;
using cgicc::HTTPContentHeader;
using cgicc::HTTPCookie;

void wiki::set_cookies(string p,string u,int time)
{
	string cookie=app.config.sval("wikipp.cookie_id","");
	HTTPCookie u_c(cookie + "username",p,"","",time,"/",false);
	set_cookie(u_c);
	HTTPCookie p_c(cookie + "password",u,"","",time,"/",false);
	set_cookie(p_c);
}

void wiki::set_options()
{
	sql<<	"DELETE FROM options "
		"WHERE lang='global' OR lang=?",
		locale,exec();
	sql<<	"INSERT INTO options(value,name,lang) "
		"VALUES(?,'users_only_edit','global')",
		ops.global.users_only_edit,exec();
	sql<<	"INSERT INTO options(value,name,lang) "
		"VALUES(?,'title',?)",
		ops.local.title,locale,exec();
	sql<<	"INSERT INTO options(value,name,lang) "
		"VALUES(?,'about',?)",
		ops.local.about,locale,exec();
	sql<<	"INSERT INTO options(value,name,lang) "
		"VALUES(?,'copyright',?)",
		ops.local.copyright,locale,exec();
	cache.rise("global_ops");
	cache.rise("local_ops:"+locale);
}

void wiki::get_options()
{
	if(!cache.fetch_data("global_ops",ops.global)) {
		sql<<	"SELECT value FROM options "
			"WHERE	lang='global' AND name='users_only_edit' ";
		row r;
		if(sql.single(r)) {
			string v;
			r >> v;
			ops.global.users_only_edit=atoi(v.c_str());
		}
		else { 
			ops.global.users_only_edit=0;
		}
		cache.store_data("global_ops",ops.global);
	}
	if(cache.fetch_data("local_ops:"+locale,ops.local))
		return;
	result res;
	sql<<	"SELECT value,name FROM options "
		"WHERE  lang=?",locale,res;
	row r;
	while(res.next(r)) {
		string v,n;
		r>>v>>n;
		if(n=="title")
			ops.local.title=v;
		else if(n=="about")
			ops.local.about=v;
		else if(n=="copyright")
			ops.local.copyright=v;
	}
	if(ops.local.title.empty())
		ops.local.title=gettext("Wiki++ &mdash; CppCMS Wiki");
	if(ops.local.about.empty())
		ops.local.about=
			gettext("## About\n"
				"\n"
				"Wiki++ is a wiki engine powered by\n"
				"[CppCMS](http://cppcms.sf.net/) web development framework.\n");
	if(ops.local.copyright.empty())
		ops.local.copyright=gettext("&copy; All Rights Reserverd");
	cache.store_data("local_ops:"+locale,ops.local);
}

void wiki::on_login()
{
	data::login c(this);
	int time=3600*24*7;
	if(env->getRequestMethod()=="POST") {
		c.form.load(*cgi);
		if(c.form.validate()) {
			redirect(locale);
			set_cookies(c.form.username.get(),c.form.password.get(),time);
			return;
		}
	}
	else {
		if(auth()) {
			redirect(env->getReferrer());
			set_cookies("","",-1);
			return;
		}
	}
	master_app.ini(c);
	render("login",c);
}

bool wiki::check_login(string u,string p)
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

bool wiki::auth()
{
	if(!auth_done)
		do_auth();
	return auth_ok;
}
void wiki::do_auth()
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

void wiki::error_forbidden()
{
	set_header(new HTTPRedirectHeader(links.admin_url(links.login).str()));
}

wiki::register_urls(string p,url_parser &u)
{
	url2.add(app,u);
}

wiki::wiki(manager const &s) :
	worker_thread(s),
	links(this),
	master_app(*this)
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
	
	string root=app.config.sval("wikipp.script");

	url.add("^/(\\w+)/admin(/.*)$",
		boost::bind(&wiki::admin,this,$1,$2));
	links.admin=root+"/%1%/admin";
	url.add("^/(\\w+)/(\\w+)(/?.*)$",
		boost::bind(&wiki::lang,this,$1,$2,$3));
	url_admin.add("/login",
		boost::bind(&wiki::on_login,this));
	links.login=links.admin+"/login";
	url_admin.add("/options",
		boost::bind(&wiki::edit_options,this));
	links.edit_options=links.admin+"/options";

	
	use_template("view");

	predefined["content"]=&wiki::content;
	links.toc=root+"/%1%/content/";
}

void wiki::edit_options()
{
	if(!auth()) {
		error_forbidden();
		return;
	}
	data::edit_options c(this);
	if(env->getRequestMethod()=="POST") {
		c.form.load(*cgi);
		if(c.form.validate()) {
			ops.global.users_only_edit=c.form.users_only.get();
			ops.local.title=c.form.wiki_title.get();
			ops.local.copyright=c.form.copyright.get();
			ops.local.about=c.form.about.get();
			set_options();
		}
	}
	else {
		c.form.users_only.set(ops.global.users_only_edit);
		c.form.wiki_title.set(ops.local.title);
		c.form.copyright.set(ops.local.copyright);
		c.form.about.set(ops.local.about);
	}
	master_app.ini(c);
	render("edit_options",c);
}

void wiki::content()
{
	data::toc c;
	master_app.ini(c);
	result res;
	sql<<	"SELECT slug,title FROM pages "
		"WHERE lang=? "
		"ORDER BY title ASC",locale,res;
	unsigned items=res.rows();
	unsigned items_left=items/3;
	unsigned items_mid=items*2/3;
	string letter="";
	row r;
	for(unsigned i=0;res.next(r);i++) {
		vector<data::toc::element> *v;
		if(i<items_left)
			v=&c.left_col;
		else if(i<items_mid)
			v=&c.middle_col;
		else 
			v=&c.right_col;

		string t,slug;
		r>>slug>>t;
		if(!t.empty() && utf8::is_valid(t.begin(),t.end()))
		{
			std::string::iterator p=t.begin();
			utf8::next(p,t.end());
			string l(t.begin(),p);
			if(letter!=l) {
				data::toc::element e;
				e.letter=l;
				v->push_back(e);
				letter=l;
			}
			data::toc::element e;
			e.title=t;
			e.url=(links.admin_url(links.page) % slug).str();
			v->push_back(e);
		}
	}
	render("toc",c);
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

void wiki::admin(string lang,string url)
{
	if(!set_locale(lang)){
		redirect();
		return;
	}
	get_options();
	if(url_admin.parse(url)<0)
		redirect(lang);
}

void wiki::lang(string lang,string url)
{
	if(!set_locale(lang)) {
		page_app.redirect();
		return;
	}
	get_options();

	if(url2.parse(url)<0) {
		page_app.redirect();
	}
}

void wiki::main()
{
	auth_done=auth_ok=false;
	if(url.parse()<0) {
		redirect();
	}
}

void wiki::page()
{
	string key="article_"+locale+":"+slug;
	if(cache.fetch_page(key))
		return;
	data::page c;

	sql<<"SELECT title,content,sidebar FROM pages WHERE lang=? AND slug=?" ,locale,slug;
	row r;
	if(!sql.single(r)) {
		string redirect=links.url(links.edit_page).str();
		set_header(new HTTPRedirectHeader(redirect));
		return;
	}
	ini_share(c);
	r >> c.title >> c.content >> c.sidebar;
	c.edit_link=links.url(links.edit_page).str();
	c.history_link=links.url(links.history).str();
	render("page",c);
	cache.store_page(key);
}


links_str::links_str(wiki *_w) : w(_w) {}

boost::format links_str::admin_url(string fmt)
{
	boost::format f(fmt);
	f % w->locale;
	return f;
}
boost::format links_str::url(string fmt)
{
	boost::format f(fmt);
	f % w->locale % w->slug;
	return f;
}

