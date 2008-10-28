#include "wiki.h"
#include <boost/bind.hpp>
#include <cgicc/HTTPRedirectHeader.h>
#include <boost/format.hpp>

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
	ini_master(c);
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

wiki::wiki(manager const &s) :
	worker_thread(s),
	links(this)
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
	url2.add("^/?$",
		boost::bind(&wiki::page,this));
	links.page=root+"/%1%/%2%";
	links.main_page=root+"/en/main";
	url2.add("^/edit/(version/(\\d+))?$",
		boost::bind(&wiki::edit_page,this,$2));
	links.edit_page=links.page+"/edit/";
	links.edit_version=links.edit_page+"version/%3%";
	url2.add("^/version/(\\d+)/?$",
		boost::bind(&wiki::page_hist,this,$1));
	links.page_hist=links.page+"/version/%3%/";
	url2.add("^/history/(\\d+)?$",
		boost::bind(&wiki::history,this,$1));
	links.history=links.page+"/history/";
	links.history_next=links.history+"%3%";
	
	use_template("view");
}

void wiki::history(string page)
{
	unsigned const vers=10;
	int offset;
	data::history c;
	ini_master(c);
	if(page.empty())
		offset=0;
	else
		offset=atoi(page.c_str());
	sql<<	"SELECT title,id FROM pages "
		"WHERE pages.lang=? AND pages.slug=? ",
		locale,slug;
	row r;
	if(!sql.single(r)) {
		redirect();
		return;
	}
	int id;
	r>>c.title>>id;
	result rs;
	sql<<	"SELECT created,version FROM history "
		"WHERE id=? "
		"ORDER BY version DESC "
		"LIMIT ?,?",
		id,offset*vers,vers+1,
		rs;
	
	if(rs.rows()>vers) {
		c.hist.resize(vers);
		c.page=(links.url(links.history_next) % (offset+1)).str() ;
	}
	else {
		c.hist.resize(rs.rows());
	}
	for(unsigned i=0;rs.next(r) && i<vers;i++) {
		int ver;
		r>>c.hist[i].update >> ver;
		c.hist[i].version=ver;
		c.hist[i].show_url=(links.url(links.page_hist) % ver).str();
		c.hist[i].edit_url=(links.url(links.edit_version) % ver).str();
	}
	c.page_link=links.url(links.page).str();
	render("history",c);
}

void wiki::page_hist(string sid)
{
	data::page_hist c;
	int id=atoi(sid.c_str());
	sql<<	"SELECT history.title,history.content,history.sidebar,history.created "
		"FROM pages "
		"JOIN history ON pages.id=history.id "
		"WHERE pages.lang=? AND pages.slug=? AND history.version=?",
			locale,slug,id;
	row r;
	if(!sql.single(r)) {
		redirect(locale,slug);
		return;
	}
	r>>c.title>>c.content>>c.sidebar>>c.date;
	c.version=id;
	c.rollback=(links.url(links.edit_version) % id).str();
	ini_share(c);
	render("page_hist",c);
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

void wiki::lang(string lang,string slug,string url)
{
	if(!set_locale(lang)) {
		redirect();
		return;
	}
	get_options();
	this->slug=slug;
	if(url2.parse(url)<0) {
		redirect(lang);
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

void wiki::redirect(string loc,string slug)
{
	string redirect=(boost::format(links.page) % loc % slug ).str();
	set_header(new HTTPRedirectHeader(redirect));
}

void wiki::save_page(int id,data::page_form &form)
{
	time_t now;
	time(&now);
	std::tm t;
	localtime_r(&now,&t);
	if(id!=-1) {
		sql<<	"INSERT INTO history(id,version,created,title,content,sidebar) "
			"SELECT id,"
			"	(SELECT COALESCE(MAX(version),0)+1 FROM history WHERE id=?),"
			"	?,title,content,sidebar from pages WHERE id=?",
				id,t,id,exec();
		sql<<	"UPDATE pages SET content=?,title=?,sidebar=?,users_only=? "
			"WHERE lang=? AND slug=?",
				form.content.get(),form.title.get(),
				form.sidebar.get(),int(form.users_only.get()),
				locale,slug,exec();
	}
	else {
		sql<<	"INSERT INTO pages(lang,slug,title,content,sidebar,users_only) "
			"VALUES(?,?,?,?,?,?)",
			locale,slug,
			form.title.get(),
			form.content.get(),
			form.sidebar.get(),
			form.users_only.get(),
			exec();
	}
}

bool wiki::load_page(data::page_form &form)
{
	sql<<	"SELECT title,content,sidebar,users_only "
		"FROM pages WHERE lang=? AND slug=?",
		locale,slug;
	row r;
	if(sql.single(r)) {
		int users_only;
		r>>form.title.str()
		 >>form.content.str()
		 >>form.sidebar.str()
		 >>users_only;
		form.users_only.set(users_only);
		return true;
	}
	form.users_only.set(ops.global.users_only_edit);
	return false;
}

bool wiki::load_history_page(int ver,data::page_form &form)
{
	sql<<	"SELECT history.title,history.content,history.sidebar,pages.users_only "
		"FROM pages "
		"JOIN history ON pages.id=history.id "
		"WHERE pages.lang=? AND pages.slug=? AND history.version=?",
		locale,slug,ver;
	row r;
	if(sql.single(r)) {
		int uonly;
		r>>form.title.str()>>form.content.str()
		 >>form.sidebar.str()>>uonly;
		 form.users_only.set(uonly);
		return true;
	}
	return false;
}

bool wiki::on_edit_post(data::edit_page &c)
{
	transaction tr(sql);
	sql<<"SELECT id,users_only FROM pages WHERE lang=? and slug=?",locale,slug;
	row r;
	int id=-1,users_only=ops.global.users_only_edit;
	if(sql.single(r)) {
		r>>id>>users_only;
	}
	if(users_only && !auth()) {
		error_forbidden();
		return false;
	}
	c.form.load(*cgi);
	if(c.form.validate()) {
		if(c.form.save.pressed || c.form.save_cont.pressed) {
			save_page(id,c.form);
			cache.rise("article_"+locale+":"+slug);
		}
		if(c.form.save.pressed) {
			redirect(locale,slug);
			tr.commit();
			return false;
		}
		if(c.form.preview.pressed) {
			c.title=c.form.title.get();
			c.content=c.form.content.get();
			c.sidebar=c.form.sidebar.get();
		}
	}
	tr.commit();
	return true;
}

void wiki::edit_page(string version)
{
	data::edit_page c(this);
	if(env->getRequestMethod()=="POST") {
		if(!on_edit_post(c))
			return;
	}
	else {
		if(version.empty()) {
			c.new_page=!load_page(c.form);
		}
		else {
			int ver=atoi(version.c_str());
			if(!load_history_page(ver,c.form)) {
				redirect(locale,slug);
				return;
			}
		}
		if(c.form.users_only.get() && !auth()) {
			error_forbidden();
		}
	}
	ini_share(c);
	c.back=links.url(links.page).str();
	render("edit_page",c);
}

void wiki::ini_master(data::master &c)
{
	c.media=app.config.sval("wikipp.media");
	c.cookie_prefix=app.config.sval("wikipp.cookie_id","");
	c.main_link=(boost::format(links.page) % locale % "main").str();
	c.login_link=links.admin_url(links.login).str();
	c.wiki_title=ops.local.title;
	c.about=ops.local.about;
	c.copyright=ops.local.copyright;
	vector<string> const &langs=app.config.slist("locale.lang_list");
	for(vector<string>::const_iterator p=langs.begin(),e=langs.end();p!=e;++p) {
		string lname;
		if(*p=="en")
			lname="English";
		else {
			/// Translate as the target language
			/// for fr gettext("LANG")="Francis"
			set_lang(*p);
			lname=gettext("LANG");
			if(lname=="LANG") {
				lname=*p;
			}
		}
		c.languages[lname]=(boost::format(links.page) % *p % "main").str();
	}
	set_lang(locale);
}
void wiki::ini_share(data::page &c)
{
	ini_master(c);
	c.edit_link=links.url(links.edit_page).str();
	c.history_link=links.url(links.history).str();
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

