#include "wiki.h"
#include <boost/bind.hpp>
#include <cgicc/HTTPRedirectHeader.h>
#include <boost/format.hpp>

using namespace dbixx;
using cgicc::HTTPRedirectHeader;
using cgicc::HTTPContentHeader;
using cgicc::HTTPCookie;

bool wiki::auth()
{
	if(!auth_done)
		do_auth();
	return auth_ok;
}
void wiki::do_auth()
{
	auth_ok=true;
	// FIXME
}

void wiki::error_forbidden()
{
	cout<<"<h1>Not Allowed</h1>";
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

	url.add("^/(\\w+)/(\\w+)(/?.*)$",
		boost::bind(&wiki::lang,this,$1,$2,$3));
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
	render("page_hist",c);
}

void wiki::lang(string lang,string slug,string url)
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
			redirect();
			return;
		}
	}
	locale=lang;
	set_lang(lang);
	this->slug=slug;
	if(url2.parse(url)<0) {
		redirect();
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

void wiki::edit_page(string version)
{
	data::edit_page c(this);
	if(env->getRequestMethod()=="POST") {
		c.form.load(*cgi);
		if(c.form.validate()) {
			if(c.form.save.pressed || c.form.save_cont.pressed) {
				time_t now;
				time(&now);
				std::tm t;
				localtime_r(&now,&t);
				transaction tr(sql);
				sql<<"SELECT id,users_only FROM pages WHERE lang=? and slug=?",locale,slug;
				row r;
				int id,users_only;
				if(sql.single(r)) {
					r>>id>>users_only;
					if(users_only && auth()) {
						error_forbidden();
						return;
					}
					sql<<	"INSERT INTO history(id,version,created,title,content,sidebar) "
						"SELECT id,"
						"	(SELECT COALESCE(MAX(version),0)+1 FROM history WHERE id=?),"
						"	?,title,content,sidebar from pages WHERE id=?",
								id,t,id,exec();
					sql<<	"UPDATE pages SET content=?,title=?,sidebar=?,users_only=? "
						"WHERE lang=? AND slug=?",
						c.form.content.get(),c.form.title.get(),
						c.form.sidebar.get(),int(c.form.users_only.get()),
						locale,slug,exec();
					tr.commit();
					cache.rise("article_"+locale+":"+slug);
							
				}
				else {
					sql<<	"INSERT INTO pages(lang,slug,title,content,sidebar,users_only) "
						"VALUES(?,?,?,?,?,?)",
						locale,slug,
						c.form.title.get(),
						c.form.content.get(),
						c.form.sidebar.get(),
						c.form.users_only.get(),
						exec();
					tr.commit();
				}

			}
			if(c.form.save.pressed) {
				redirect(locale,slug);
				return;
			}
			if(c.form.preview.pressed) {
				c.title=c.form.title.get();
				c.content=c.form.content.get();
				c.sidebar=c.form.sidebar.get();
			}
		}
	}
	else {
		if(version.empty()) {
			sql<<"SELECT title,content,sidebar,users_only FROM pages WHERE lang=? AND slug=?",locale,slug;
			row r;
			if(sql.single(r)) {
				int users_only;
				r>>c.form.title.str()>>c.form.content.str()>>c.form.sidebar.str()>>users_only;
				c.form.users_only.set(users_only);
			}
			else {
				c.new_page=true;
			}
		}
		else {
			int ver=atoi(version.c_str());
			sql<<	"SELECT history.title,history.content,history.sidebar,pages.users_only "
				"FROM pages "
				"JOIN history ON pages.id=history.id "
				"WHERE pages.lang=? AND pages.slug=? AND history.version=?",
				locale,slug,ver;
			row r;
			if(sql.single(r)) {
				int uonly;
				r>>c.form.title.str()>>c.form.content.str()>>c.form.sidebar.str()>>uonly;
				c.form.users_only.set(uonly);
			}
			else {
				redirect(locale,slug);
				return;
			}

		}
	}
	ini_share(c);
	render("edit_page",c);
}

void wiki::ini_share(data::page &c)
{
	c.media=app.config.sval("wikipp.media");
	c.edit_link=links.url(links.edit_page).str();
	c.history_link=links.url(links.history).str();

}

links_str::links_str(wiki *_w) : w(_w) {}

boost::format links_str::url(string fmt)
{
	boost::format f(fmt);
	f % w->locale % w->slug;
	return f;
}

