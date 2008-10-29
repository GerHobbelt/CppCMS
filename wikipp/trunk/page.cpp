#include <cgicc/HTTPRedirectHeader.h>
#include "page.h"
#include "wiki.h"

using namespace dbixx;

using cgicc::HTTPRedirectHeader;

namespace apps {

page::page(wiki &w):
	master(w)
{
	wi.url_next.add("^/page/(\\w+)/version/(\\d+)$",
		boost::bind(&page::display_ver,this,$1,$2));
	wi.url_next.add("^/page/(\\w+)/?$",
		boost::bind(&page::display,this,$1));
	wi.url_next.add("^/page/(\\w+)/edit(/version/(\\d+))?$",
		boost::bind(&page::edit,this,$1,$3));
	wi.url_next.add("^/page/(\\w+)/history(/|(/\\d+))?$",
		boost::bind(&page::history,this,$1,$3));
}

string page::page_url(string l,string s)
{
	if(l.empty()) l=locale;
	if(s.empty()) s=slug;
	return wi.root(l)+"/page/"+s;
}

string page::page_version_url(int ver)
{
	return wi.root()+
		(boost::format("/page/%1%/version/%2%") % slug % ver).str();
}
string page::edit_url()
{
	return wi.root()+"/page/"+slug+"/edit";
}
string page::edit_version_url(int ver)
{
	return (boost::format(edit_url()+"/version/%1%") % ver).str();
}
string page::history_url(int n)
{
	string u=wi.root()+"/page/"+slug+"/history/";
	if(n)
		u+=(boost::format("%1%") % n).str();
	return u;
}

void page::history(string slug,string page)
{
	this->slug=slug;
	unsigned const vers=10;
	int offset;
	data::history c;
	master::ini(c);
	if(page.empty())
		offset=0;
	else
		offset=atoi(page.c_str());

	wi.sql<<"SELECT title,id FROM pages "
		"WHERE pages.lang=? AND pages.slug=? ",
		locale,slug;
	row r;
	if(!wi.sql.single(r)) {
		redirect(locale);
		return;
	}
	int id;
	r>>c.title>>id;
	result rs;
	wi.sql<<"SELECT created,version FROM history "
		"WHERE id=? "
		"ORDER BY version DESC "
		"LIMIT ?,?",
		id,offset*vers,vers+1,
		rs;
	
	if(rs.rows()>vers) {
		c.hist.resize(vers);
		c.page=history_url(offset+1);
	}
	else {
		c.hist.resize(rs.rows());
	}

	for(unsigned i=0;rs.next(r) && i<vers;i++) {
		int ver;
		r>>c.hist[i].update >> ver;
		c.hist[i].version=ver;
		c.hist[i].show_url=page_version_url(ver);
		c.hist[i].edit_url=edit_version_url(ver);
	}

	c.page_link=history_url(offset+1);
	wi.render("history",c);
}


void page::display(string slug)
{
	this->slug=slug;
	string key="article_"+locale+":"+slug;
	if(wi.cache.fetch_page(key))
		return;
	data::page c;

	wi.sql<<"SELECT title,content,sidebar FROM pages WHERE lang=? AND slug=?",
		locale,slug;
	row r;
	if(!wi.sql.single(r)) {
		string redirect=edit_url();
		wi.set_header(new HTTPRedirectHeader(redirect));
		return;
	}
	ini(c);
	r >> c.title >> c.content >> c.sidebar;
	wi.render("page",c);
	wi.cache.store_page(key);
}

void page::ini(data::page &c)
{
	master::ini(c);
	c.edit_link=edit_url();
	c.history_link=history_url();
}

void page::edit(string slug,string version)
{
	this->slug=slug;
	data::edit_page c(&wi);
	if(wi.env->getRequestMethod()=="POST") {
		if(!edit_on_post(c))
			return;
	}
	else {
		if(version.empty()) {
			c.new_page=!load(c.form);
		}
		else {
			int ver=atoi(version.c_str());
			if(!load_history(ver,c.form)) {
				redirect(locale,slug);
				return;
			}
		}
		if(c.form.users_only.get() && !wi.users.auth()) {
			wi.users.error_forbidden();
		}
	}
	ini(c);
	c.back=page_url();
	wi.render("edit_page",c);
}

bool page::load(data::page_form &form)
{
	wi.sql<<
		"SELECT title,content,sidebar,users_only "
		"FROM pages WHERE lang=? AND slug=?",
		locale,slug;
	row r;
	if(wi.sql.single(r)) {
		int users_only;
		r>>form.title.str()
		 >>form.content.str()
		 >>form.sidebar.str()
		 >>users_only;
		form.users_only.set(users_only);
		return true;
	}
	wi.options.load();
	form.users_only.set(wi.options.global.users_only_edit);
	return false;
}

void page::redirect(string loc,string slug)
{
	string redirect=page_url(loc,slug);
	wi.set_header(new HTTPRedirectHeader(redirect));
}

void page::save(int id,data::page_form &form)
{
	time_t now;
	time(&now);
	std::tm t;
	localtime_r(&now,&t);
	if(id!=-1) {
		wi.sql<<"INSERT INTO history(id,version,created,title,content,sidebar) "
			"SELECT id,"
			"	(SELECT COALESCE(MAX(version),0)+1 FROM history WHERE id=?),"
			"	?,title,content,sidebar from pages WHERE id=?",
				id,t,id,exec();
		wi.sql<<"UPDATE pages SET content=?,title=?,sidebar=?,users_only=? "
			"WHERE lang=? AND slug=?",
				form.content.get(),form.title.get(),
				form.sidebar.get(),int(form.users_only.get()),
				locale,slug,exec();
	}
	else {
		wi.sql<<"INSERT INTO pages(lang,slug,title,content,sidebar,users_only) "
			"VALUES(?,?,?,?,?,?)",
			locale,slug,
			form.title.get(),
			form.content.get(),
			form.sidebar.get(),
			form.users_only.get(),
			exec();
	}
}


bool page::edit_on_post(data::edit_page &c)
{
	wi.options.load();
	
	transaction tr(wi.sql);
	wi.sql<<"SELECT id,users_only FROM pages WHERE lang=? and slug=?",locale,slug;
	row r;
	int id=-1,users_only=wi.options.global.users_only_edit;
	if(wi.sql.single(r)) {
		r>>id>>users_only;
	}
	if(users_only && !wi.users.auth()) {
		wi.users.error_forbidden();
		return false;
	}
	c.form.load(*wi.cgi);
	if(c.form.validate()) {
		if(c.form.save.pressed || c.form.save_cont.pressed) {
			save(id,c.form);
			wi.cache.rise("article_"+locale+":"+slug);
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

bool page::load_history(int ver,data::page_form &form)
{
	wi.sql<<"SELECT history.title,history.content,history.sidebar,pages.users_only "
		"FROM pages "
		"JOIN history ON pages.id=history.id "
		"WHERE pages.lang=? AND pages.slug=? AND history.version=?",
		locale,slug,ver;
	row r;
	if(wi.sql.single(r)) {
		int uonly;
		r>>form.title.str()>>form.content.str()
		 >>form.sidebar.str()>>uonly;
		 form.users_only.set(uonly);
		return true;
	}
	return false;
}

void page::display_ver(string slug,string sid)
{
	this->slug=slug;
	data::page_hist c;
	int id=atoi(sid.c_str());
	wi.sql<<"SELECT history.title,history.content,history.sidebar,history.created "
		"FROM pages "
		"JOIN history ON pages.id=history.id "
		"WHERE pages.lang=? AND pages.slug=? AND history.version=?",
			locale,slug,id;
	row r;
	if(!wi.sql.single(r)) {
		redirect(locale,slug);
		return;
	}
	r>>c.title>>c.content>>c.sidebar>>c.date;
	c.version=id;
	c.rollback=edit_version_url(id);
	ini(c);
	wi.render("page_hist",c);
}

}


