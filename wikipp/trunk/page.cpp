#include <cgicc/HTTPRedirectHeader.h>
#include "page.h"
#include "page_data.h"
#include "wiki.h"
#include "diff.h"

using namespace dbixx;

using cgicc::HTTPRedirectHeader;

namespace data { 
// Page data
page_form::page_form(wiki *_w):
	w(_w),
	title("title",w->gettext("Title")),
	content("content",w->gettext("Content")),
	sidebar("sidebar",w->gettext("Sidebar")),
	save("save",w->gettext("Save")),
	save_cont("save_cont",w->gettext("Save and Continue")),
	preview("preview",w->gettext("Preview")),
	users_only("users_only")
{
	*this & title & content & sidebar & save & save_cont & preview & users_only;
	fields<<title<<content<<sidebar;
	buttons<<save<<save_cont<<preview<<users_only;
	users_only.help=w->gettext("Disable editing by visitors");
	users_only.error_msg=w->gettext("Please Login");
	title.set_nonempty();
	content.set_nonempty();
	content.rows=25;
	content.cols=60;
	sidebar.rows=10;
	sidebar.cols=60;
}

bool page_form::validate()
{
	bool res=form::validate();
	if(users_only.get() && !w->users.auth()) {
		users_only.not_valid();
		users_only.set(false);
		return false;
	}
	return res;
}

} // namespace data

namespace apps {

page::page(wiki &w):
	master(w)
{
	wi.url_next.add("^/page/(\\w+)/version/(\\d+)$",
		boost::bind(&page::display_ver,this,_1,_2));
	wi.url_next.add("^/page/(\\w+)/?$",
		boost::bind(&page::display,this,_1));
	wi.url_next.add("^/page/(\\w+)/edit(/version/(\\d+))?$",
		boost::bind(&page::edit,this,_1,_3));
	wi.url_next.add("^/page/(\\w+)/history(/|/(\\d+))?$",
		boost::bind(&page::history,this,_1,_3));
	wi.url_next.add("^/page/(\\w+)/diff/(\\d+)vs(\\d+)/?$",
		boost::bind(&page::diff,this,_1,_2,_3));
}

string page::diff_url(int v1,int v2,string l,string s)
{
	if(l.empty()) l=locale;
	if(s.empty()) s=slug;
	return wi.root(l) + 
		(boost::format("/page/%1%/diff/%2%vs%3%") % s % v1 % v2).str();
}

string page::page_url(string l,string s)
{
	if(l.empty()) l=locale;
	if(s.empty()) s=slug;
	return wi.root(l)+"/page/"+s;
}

string page::page_version_url(int ver,string l,string s)
{
	if(l.empty()) l=locale;
	if(s.empty()) s=slug;
	return wi.root(l)+
		(boost::format("/page/%1%/version/%2%") % s % ver).str();
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

void page::diff(string slug,string sv1,string sv2)
{
	int v1=atoi(sv1.c_str()), v2=atoi(sv2.c_str());
	this->slug=slug;
	result rs;
	data::diff c;
	c.v1=v1;
	c.v2=v2;
	c.edit_v1=edit_version_url(v1);
	c.edit_v2=edit_version_url(v2);
	sql<<	"SELECT version,history.title,history.content,history.sidebar,pages.title FROM pages "
		"JOIN history ON pages.id=history.id "
		"WHERE lang=? AND slug=? AND version IN (?,?) ",
		locale,slug,v1,v2,rs;
	if(rs.rows()!=2) {
		c.no_versions=true;
		master::ini(c);
		render("diff",c);
		return;
	}
	string t1,c1,s1,t2,c2,s2;
	row r;
	while(rs.next(r)) {
		int ver;
		r>>ver;
		if(ver==v1) {
			r>>t1>>c1>>s1>>c.title;
		}
		else {
			r>>t2>>c2>>s2;
		}
	}
	if(t1!=t2) {
		c.title_diff=true;
		c.title_1=t1;
		c.title_2=t2;
	}
	else {
		c.title=t1;
	}
	if(c1!=c2) {
		c.content_diff=true;
		vector<string> X=split(c1);
		vector<string> Y=split(c2);
		diff::diff(X,Y,c.content_diff_data);
	}
	if(s1!=s2) {
		c.sidebar_diff=true;
		vector<string> X=split(s1);
		vector<string> Y=split(s2);
		diff::diff(X,Y,c.sidebar_diff_data);
	}
	if(t1==t2 && c1==c2 && s1==s2) 
		c.no_diff=true;
	master::ini(c);
	render("diff",c);
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

	sql<<	"SELECT title,id FROM pages "
		"WHERE pages.lang=? AND pages.slug=? ",
		locale,slug;
	row r;
	if(!sql.single(r)) {
		redirect(locale);
		return;
	}
	int id;
	r>>c.title>>id;
	result rs;
	sql<<	"SELECT created,version,author FROM history "
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
		r>>c.hist[i].update >> ver >> c.hist[i].author ;
		c.hist[i].version=ver;
		c.hist[i].show_url=page_version_url(ver);
		c.hist[i].edit_url=edit_version_url(ver);
		if(ver>1)
			c.hist[i].diff_url=diff_url(ver-1,ver);
	}

	c.page_link=page_url();
	render("history",c);
}


void page::display(string slug)
{
	this->slug=slug;
	string key="article_"+locale+":"+slug;
	if(cache.fetch_page(key))
		return;
	data::page c;

	sql<<	"SELECT title,content,sidebar FROM pages WHERE lang=? AND slug=?",
		locale,slug;
	row r;
	if(!sql.single(r)) {
		string redirect=edit_url();
		set_header(new HTTPRedirectHeader(redirect));
		add_header("Status: 302 Found");
		return;
	}
	ini(c);
	r >> c.title >> c.content >> c.sidebar;
	render("page",c);
	cache.store_page(key);
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
	if(env->getRequestMethod()=="POST") {
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
	c.submit=edit_url();
	render("edit_page",c);
}

bool page::load(data::page_form &form)
{
	sql<<	
		"SELECT title,content,sidebar,users_only "
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
	wi.options.load();
	form.users_only.set(wi.options.global.users_only_edit);
	return false;
}

void page::redirect(string loc,string slug)
{
	string redirect=page_url(loc,slug);
	set_header(new HTTPRedirectHeader(redirect));
	add_header("Status: 302 Found");
}

void page::save(int id,data::page_form &form)
{
	time_t now;
	time(&now);
	std::tm t;
	localtime_r(&now,&t);
	wi.users.auth();
	if(id!=-1) {
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
		id=sql.rowid();
	}
	sql<<	"INSERT INTO history(id,version,created,title,content,sidebar,author) "
		"SELECT ?,"
		"	(SELECT COALESCE(MAX(version),0)+1 FROM history WHERE id=?),"
		"	?,?,?,?,?",
			id,id,t,
			form.title.get(),
			form.content.get(),
			form.sidebar.get(),
			wi.users.username,
			exec();
}


bool page::edit_on_post(data::edit_page &c)
{
	wi.options.load();
	
	transaction tr(sql);
	sql<<	"SELECT id,users_only FROM pages WHERE lang=? and slug=?",locale,slug;
	row r;
	int id=-1,users_only=wi.options.global.users_only_edit;
	if(sql.single(r)) {
		r>>id>>users_only;
	}
	if(users_only && !wi.users.auth()) {
		wi.users.error_forbidden();
		return false;
	}
	c.form.load(*cgi);
	if(c.form.validate()) {
		if(c.form.save.pressed || c.form.save_cont.pressed) {
			save(id,c.form);
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

bool page::load_history(int ver,data::page_form &form)
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

void page::display_ver(string slug,string sid)
{
	this->slug=slug;
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
	c.rollback=edit_version_url(id);
	ini(c);
	render("page_hist",c);
}

}


