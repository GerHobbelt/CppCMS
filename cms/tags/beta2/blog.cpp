#include "blog.h"
#include <time.h>
#include <cppcms/text_tool.h>
#include <boost/format.hpp>
#include <cgicc/HTTPRedirectHeader.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "error.h"
#include <cppcms/text_tool.h>
#include "cxxmarkdown/markdowncxx.h"
#include "md5.h"

using cgicc::FormEntry;
using cgicc::HTTPContentHeader;
using cgicc::HTTPRedirectHeader;
using cgicc::HTTPCookie;
using namespace dbixx;
using boost::format;
using boost::str;
using namespace tmpl;

struct In_Comment {
	string message;
	string author;
	string url;
	string email;
	bool load(const vector<FormEntry> &form);
};

bool In_Comment::load(const vector<FormEntry> &form)
{
	unsigned i;
	for(i=0;i<form.size();i++) {
		string const &field=form[i].getName();
		if(field=="username") {
			author=form[i].getValue();
		}
		else if(field=="email") {
			email=form[i].getValue();
		}
		else if(field=="url") {
			url=form[i].getValue();
		}
		else if(field=="message") {
			message=form[i].getValue();
		}
	}
	if(message.size()==0 || author.size()==0 || email.size()==0) {
		return false;
	}
	return true;
}

static void old_markdown2html(string const &in,string &out)
{
	Text_Tool tt;
	string tmp;
	tt.markdown2html(in.c_str(),tmp);
	out.append(tmp);
}

static void create_gif(string const &tex,string const &fname)
{
	int pid;
	string p=global_config.sval("latex.script");
	if((pid=fork())<0) {
		char buffer[256];
		strerror_r(errno,buffer,sizeof(buffer));
		throw std::runtime_error(string("Failed to fork! ")+buffer);
	}
	else if(pid==0) {
		if(execl(p.c_str(),p.c_str(),tex.c_str(),fname.c_str(),NULL)<0) {
			perror("exec");
			exit(1);
		}
	}
	else {
		int stat;
		waitpid(pid,&stat,0);
	}
}

static void latex_filter(string const &in,string &out)
{
	string::size_type p_old=0,p1=0,p2=0;
	while((p1=in.find("[tex]",p_old))!=string::npos && (p2=in.find("[/tex]",p_old))!=string::npos && p2>p1) {
		out.append(in,p_old,p1-p_old);
		p1+=5;
		string tex;
		tex.append(in,p1,p2-p1);
		char md5[33];
		md5_onepass(tex.c_str(),tex.size(),md5);
		string file=global_config.sval("latex.cache_path")+ md5 +".gif";
		string wwwfile=global_config.sval("blog.media_path")+"/latex/" + md5 +".gif";
		struct stat tmp;
		if(stat(file.c_str(),&tmp)<0) {
			create_gif(tex,file);
		}
		string html_tex;
		Text_Tool tt;
		tt.text2html(tex,html_tex);
		out.append(str(boost::format("<img src=\"%1%\" alt=\"%2%\" align=\"absmiddle\" />") % wwwfile % html_tex));
		p_old=p2+6;
	}
	out.append(in,p_old,in.size()-p_old);
}

void Blog::init()
{
	string root=global_config.sval("blog.script_path");

	fmt.media=global_config.sval("blog.media_path");

	if(global_config.lval("blog.configure",0)==1) {
		url.add("^/?$",
			boost::bind(&Blog::setup_blog,this));
		fmt.admin=root + "/";
	}
	else {
		url.add("^/?$",
			boost::bind(&Blog::main_page,this,"end","all"));
		fmt.main=root + "/";
		url.add("^/from/(\\d+)$",
			boost::bind(&Blog::main_page,this,$1,"all"));
		fmt.main_from=root + "/from/%1%";

		url.add("^/cat/(\\d+)$",
			boost::bind(&Blog::main_page,this,"end",$1));
		fmt.cat=root + "/cat/%1%";

		url.add("^/cat/(\\d+)/from/(\\d+)$",
			boost::bind(&Blog::main_page,this,$2,$1));
		fmt.cat_from=root + "/cat/%1%/from/%2%";


		url.add("^/post/(\\d+)$",
			boost::bind(&Blog::post,this,$1,false));
		fmt.post=root + "/post/%1%";

		url.add("^/page/(\\d+)$",
			boost::bind(&Blog::page,this,$1,false));
		fmt.page=root + "/page/%1%";

		url.add("^/page/preview/(\\d+)$",
			boost::bind(&Blog::page,this,$1,true));
		fmt.page_preview=root + "/page/preview/%1%";

		url.add("^/post/preview/(\\d+)$",
			boost::bind(&Blog::post,this,$1,true));
		fmt.preview=root + "/post/preview/%1%";

		url.add("^/admin$",
			boost::bind(&Blog::admin,this));
		fmt.admin=root + "/admin";

		url.add("^/admin/new_page$",
			boost::bind(&Blog::edit_post,this,"new","page"));
		fmt.new_page=root + "/admin/new_page";

		url.add("^/admin/edit_page/(\\d+)$",
			boost::bind(&Blog::edit_post,this,$1,"page"));
		fmt.edit_page=root+"/admin/edit_page/%1%";

		url.add("^/admin/new_post$",
			boost::bind(&Blog::edit_post,this,"new","post"));
		fmt.new_post=root + "/admin/new_post";

		url.add("^/admin/edit_post/(\\d+)$",
			boost::bind(&Blog::edit_post,this,$1,"post"));
		fmt.edit_post=root+"/admin/edit_post/%1%";

		url.add("^/admin/edit_comment/(\\d+)$",
			boost::bind(&Blog::edit_comment,this,$1));
		fmt.edit_comment=root+"/admin/edit_comment/%1%";

		// All incoming information

		url.add("^/postback/comment/(\\d+)$",
			boost::bind(&Blog::add_comment,this,$1));
		fmt.add_comment=root+"/postback/comment/%1%";

		url.add("^/postback/page/new$",
			boost::bind(&Blog::get_post,this,"new","page"));
		fmt.add_page=root+"/postback/page/new";

		url.add("^/postback/page/(\\d+)$",
			boost::bind(&Blog::get_post,this,$1,"page"));
		fmt.update_page=root+"/postback/page/%1%";

		url.add("^/postback/post/new$",
			boost::bind(&Blog::get_post,this,"new","post"));
		fmt.add_post=root+"/postback/post/new";

		url.add("^/postback/post/(\\d+)$",
			boost::bind(&Blog::get_post,this,$1,"post"));
		fmt.update_post=root+"/postback/post/%1%";

		url.add("^/postback/update_comment/(\\d+)$",
			boost::bind(&Blog::update_comment,this,$1));
		fmt.update_comment=root+"/postback/update_comment/%1%";

		url.add("^/admin/login$",
			boost::bind(&Blog::login,this));
		fmt.login=root+"/admin/login";

		url.add("^/admin/options?$",
			boost::bind(&Blog::edit_options,this));
		fmt.edit_options=root + "/admin/options";

		url.add("^/admin/links?$",
			boost::bind(&Blog::edit_links,this));
		fmt.edit_links=root + "/admin/links";

		url.add("^/admin/categories?$",
			boost::bind(&Blog::edit_cats,this));
		fmt.edit_cats=root + "/admin/categories";


		url.add("^/admin/logout$",
			boost::bind(&Blog::logout,this));
		fmt.logout=root+"/admin/logout";

		url.add("^/postback/delete/comment/(\\d+)$",
			boost::bind(&Blog::del_comment,this,$1));
		fmt.del_comment=
			root+"/postback/delete/comment/%1%";
		url.add("^/rss$",boost::bind(&Blog::feed,this,"all"));
		fmt.feed=root+"/rss";

		url.add("^/rss/cat/(\\d+)$",boost::bind(&Blog::feed,this,$1));
		fmt.feed_cats=root+"/rss/cat/%1%";

		url.add("^/rss/comments$",boost::bind(&Blog::feed_comments,this));
		fmt.feed_comments=root+"/rss/comments";

	}

	try {
		string engine=global_config.sval("dbi.engine");
		sql.driver(engine);
		if(engine=="sqlite3") {
			sql.param("dbname",global_config.sval("sqlite3.db"));
			sql.param("sqlite3_dbdir",global_config.sval("sqlite3.dir"));
		}
		else if(engine=="mysql") {
			sql.param("dbname",global_config.sval("mysql.db"));
			sql.param("username",global_config.sval("mysql.user"));
			sql.param("password",global_config.sval("mysql.pass"));
		}
		else if(engine=="pgsql") {
			sql.param("dbname",global_config.sval("pgsql.db"));
			sql.param("username",global_config.sval("pgsql.user"));
		}
		sql.connect();
	}
	catch(dbixx_error const &e){
		throw HTTP_Error(string("Failed to access DB")+e.what());
	}
	connected=true;
	string e;
	if((e=global_config.sval("markdown.engine","discount"))=="discount"){
		render.add_string_filter("markdown2html",
			boost::bind(markdown2html,_1,_2));
	}
	else {
		render.add_string_filter("markdown2html",
			boost::bind(old_markdown2html,_1,_2));
	}
	render.add_string_filter("latex",
		boost::bind(latex_filter,_1,_2));


}


void Blog::page(string s_id,bool preview)
{
	int id=atoi(s_id.c_str());

	View_Main_Page view(this,c);
 	view.ini_page(id,preview);

	render.render(c,"master",out.getstring());
}

void Blog::edit_options()
{
	auth_or_throw();

	const vector<FormEntry> &form=cgi->getElements();

	string name,desc,copyright,contact;

	unsigned i;

	for(i=0;i<form.size();i++) {
		string const &field=form[i].getName();
		if(field=="name") {
			name=form[i].getValue();
		}
		else if(field=="desc") {
			desc=form[i].getValue();
		}
		else if(field=="contact") {
			contact=form[i].getValue();
		}
		else if(field=="copyright") {
			copyright=form[i].getValue();
		}
	}

	if(form.size()>0){
		transaction tr(sql);
		sql<<"DELETE FROM text_options WHERE id='copyright'",exec();
		sql<<"DELETE FROM options WHERE id=? OR id=? OR id=?",
			BLOG_TITLE,BLOG_DESCRIPTION,BLOG_CONTACT,exec();
		if(copyright!=""){
			sql<<	"INSERT INTO text_options(id,value) "
				"VALUES('copyright',?)",
					copyright,exec();
		}
		sql<<"INSERT INTO options(id,value) VALUES(?,?)",
			BLOG_TITLE,name,exec();
		sql<<"INSERT INTO options(id,value) VALUES(?,?)",
			BLOG_DESCRIPTION,desc,exec();
		sql<<"INSERT INTO options(id,value) VALUES(?,?)",
			BLOG_CONTACT,contact,exec();
		tr.commit();
	}

	View_Admin view(this,c);
	view.ini_options();
	render.render(c,"admin",out.getstring());
}

void Blog::edit_links()
{
	auth_or_throw();

	const vector<FormEntry> &form=cgi->getElements();

	if(form.size()>0) {
		unsigned i;
		int id=-1,cat_id=-1;
		string url,name,descr,type;
		for(i=0;i<form.size();i++) {
			string const &field=form[i].getName();
			if(field=="del_link") {
				id=form[i].getIntegerValue();
				sql<<"DELETE FROM links WHERE id=?",id,exec();
				break;
			}
			if(field=="del_cat") {
				id=form[i].getIntegerValue();
				row r;
				int num;
				transaction tr(sql);
				sql<<"SELECT count(*) from links WHERE cat_id=?",id;
				sql.single(r);
				r>>num;
				if(num==0){
					sql<<"DELETE FROM link_cats WHERE id=?",id,exec();
					tr.commit();
				}
				else {
					c["not_empty"]=true;
				}
				break;
			}
			else if(field=="id") {
				id=form[i].getIntegerValue();
			}
			else if(field=="cat") {
				cat_id=form[i].getIntegerValue();
			}
			else if(field=="type") {
				type=form[i].getValue();
			}
			else if(field=="name") {
				name=form[i].getValue();
			}
			else if(field=="url") {
				url=form[i].getValue();
			}
			else if(field=="descr") {
				descr=form[i].getValue();
			}
		}
		if(type=="newcat" && name!="") {
			sql<<"INSERT INTO link_cats(name) values(?)",name,exec();
		}
		else if(type=="newlink" && cat_id!=-1 && name!="" && url!="") {
			transaction tr(sql);
			sql<<"SELECT id FROM link_cats WHERE id=?",cat_id;
			row r;
			if(sql.single(r)){
				sql<<	"INSERT INTO links(cat_id,title,url,description) "
					"VALUES(?,?,?,?)",cat_id,name,url,descr,exec();
				tr.commit();
			}
		}
		else if(type=="cat" && id!=-1) {
			sql<<"UPDATE link_cats SET name=? WHERE id=?",name,id,exec();
		}
		else if(type=="link" && id!=-1) {
			transaction tr(sql);
			sql<<"SELECT id FROM link_cats WHERE id=?",cat_id;
			row r;
			if(sql.single(r)){
				sql<<"UPDATE links SET cat_id=?,title=?,url=?,description=? WHERE id=?",
					cat_id,name,url,descr,id,exec();
				tr.commit();
			}
		}
	}

	View_Admin view(this,c);
	view.ini_links();
	render.render(c,"admin",out.getstring());
}



void Blog::edit_cats()
{
	auth_or_throw();

	c["constraint_error"]=false;

	const vector<FormEntry> &form=cgi->getElements();

	if(form.size()>0) {
		unsigned i;
		int id=-1;
		string name;
		bool recur=false;
		bool del=false;
		bool new_cat=false;
		for(i=0;i<form.size();i++) {
			string const &field=form[i].getName();
			if(field=="id") {
				id=form[i].getIntegerValue();
			}
			else if(field=="name") {
				name=form[i].getValue();
			}
			else if(field=="new") {
				new_cat=true;
			}
			else if(field=="delete") {
				del=true;
			}
			else if(field=="recur") {
				recur=true;
			}
		}
		if(new_cat && name!="") {
			sql<<"INSERT INTO cats(name) VALUES(?)",name,exec();
		}
		else if(del && id!=-1) {
			transaction tr(sql);
			if(recur) {
				sql<<"DELETE FROM post2cat WHERE cat_id=?",id,exec();
				sql<<"DELETE FROM cats WHERE id=?",id,exec();
				tr.commit();
			}
			else {
				row r;
				sql<<"SELECT post_id FROM post2cat WHERE cat_id=? LIMIT 1",id;
				if(sql.single(r)) {
					c["constraint_error"]=true;
					tr.rollback();
				}
				else {
					sql<<"DELETE FROM cats WHERE id=?",id,exec();
					tr.commit();
				}
			}
		}
		else if(id!=-1 && name!="") {
			sql<<"UPDATE cats SET name=? WHERE id=?",name,id,exec();
		}
	}

	View_Admin view(this,c);
	view.ini_cats();
	render.render(c,"admin",out.getstring());
}




void Blog::post(string s_id,bool preview)
{
	if(preview) {
		auth_or_throw();
	}
	int id=atoi(s_id.c_str());

	View_Main_Page view(this,c);
	view.ini_post(id,preview);

	render.render(c,"master",out.getstring());
}

void Blog::main_page(string from,string cat)
{

	View_Main_Page view(this,c);
	int pid=from=="end" ? -1 : atoi(from.c_str());
	int cid=cat =="all" ? -1 : atoi(cat.c_str());

	view.ini_main(pid,false,cid);

	render.render(c,"master",out.getstring());
}

void Blog::date(tm t,string &d)
{
	char buf[80];
	snprintf(buf,80,"%02d/%02d/%04d, %02d:%02d",
		 t.tm_mday,t.tm_mon+1,t.tm_year+1900,
		 t.tm_hour,t.tm_min);
	d=buf;
}

void Blog::set_lang()
{
	if(global_config.lval("locale.gnugettext",0)==1) {
		render.set_translator(gnugt);
		return;
	}
	string default_locale=global_config.sval("locale.default","en");

	if(global_config.lval("locale.multiple",0)==0) {
		render.set_translator(tr[default_locale]);
	}
	else {
		const vector<HTTPCookie> &cookies = env->getCookieList();
		unsigned i;
		render.set_translator(tr[default_locale]);
		for(i=0;i!=cookies.size();i++) {
			if(cookies[i].getName()=="lang") {
				string lang=cookies[i].getValue();
				render.set_translator(tr[lang]);
				break;
			}
		}

		content::vector_t &langlist=c.vector("langlist",tr.get_names().size());
		map<string,string>::const_iterator p;
		map<string,string> const &names=tr.get_names();
		for(i=0,p=names.begin();p!=names.end();p++,i++) {
			langlist[i]["code"]=p->first;
			langlist[i]["name"]=p->second;
		}

	}
}

void Blog::main()
{
	try {
		try{
			if(!connected)
				sql.reconnect();
			c.clear();
			auth();
			set_lang();
			if(url.parse()==-1){
				throw Error(Error::E404);
			}
		}
		catch(Error &e) {
			error_page(e.what());
		}
	}
	catch (dbixx_error &err) {
		sql.close();
		connected=false;
		string error_message=err.what();
		if(global_config.lval("dbi.debug",0)==1) {
			error_message+=":";
//			error_message+=err.query();
		}
		throw HTTP_Error(error_message);
	}
	catch(char const *s) {
		throw HTTP_Error(s);
	}

}

void Blog::add_comment(string &postid)
{
	int post_id=atoi(postid.c_str());
	In_Comment incom;
	if(!incom.load(cgi->getElements())) {
		throw Error(Error::COMMENT_FIELDS);
	}

	post_t post;
	post.is_open=0;

	transaction tr(sql);

	row r;
	sql<<"SELECT is_open FROM posts WHERE id=?",
		post_id;
	if(!sql.single(r) || (r>>post.is_open , !post.is_open) ) {
		throw Error(Error::E404);
	}

	tm t;
	time_t cur=time(NULL);
	localtime_r(&cur,&t);

	sql<<	"INSERT INTO "
		"comments (post_id,author,url,email,publish_time,content) "
		"values(?,?,?,?,?,?)",
		post_id,incom.author,incom.url,
		incom.email,t,incom.message;
	sql.exec();
	count_comments(post_id);
	tr.commit();

	string redirect=str(format(fmt.post) % post_id);
	set_header(new HTTPRedirectHeader(redirect));
}
void Blog::count_comments(int id)
{
	sql<<	"UPDATE posts "
		"SET	comment_count=(SELECT count(*) FROM comments WHERE post_id=?) "
		"WHERE	id=?",id,id,exec();
}

void Blog::error_page(int what)
{
	if(what==Error::AUTH) {
		View_Admin view(this,c);
 		view.ini_login();
		render.render(c,"admin",out.getstring());
	}
	else {
		View_Main_Page view(this,c);
		view.ini_error(what);
		render.render(c,"master",out.getstring());
	}
}

int Blog::check_login( string username,string password)
{
	int id=-1;
	string pass;
	if(username==""){
		return -1;
	}
	row r;
	sql<<	"SELECT id,password FROM users WHERE username=?",
		username;
	if(!sql.single(r))
		return -1;

	r>>id>>pass;

	if(id!=-1 && password==pass) {
		return id;
	}
	return -1;
}

bool Blog::auth()
{
	int id;
	unsigned i;
	string tmp_username;
	string tmp_password;

	const vector<HTTPCookie> &cookies = env->getCookieList();

	for(i=0;i!=cookies.size();i++) {
		if(cookies[i].getName()=="username") {
			tmp_username=cookies[i].getValue();
		}
		else if(cookies[i].getName()=="password") {
			tmp_password=cookies[i].getValue();
		}
	}
	if((id=check_login(tmp_username,tmp_password))!=-1) {
		this->username=tmp_username;
		this->userid=id;
		return true;
	}
	this->username="";
	this->userid=-1;
	return false;
}

void Blog::auth_or_throw()
{
	if(userid==-1){
		throw Error(Error::AUTH);
	}
}

void Blog::set_login_cookies(string u,string p,int d)
{
	if(d<0) {
		d=-1;
	}
	else {
		d*=24*3600;
	}
	HTTPCookie u_c("username",u,"","",d,"/",false);
	response_header->setCookie(u_c);
	HTTPCookie p_c("password",p,"","",d,"/",false);
	response_header->setCookie(p_c);
}

void Blog::login()
{
	unsigned i;
	string tmp_username,tmp_password;
	const vector<FormEntry> &form=cgi->getElements();

	for(i=0;i<form.size();i++) {
		string const &field=form[i].getName();
		if(field=="username") {
			tmp_username=form[i].getValue();
		}
		else if(field=="password") {
			tmp_password=form[i].getValue();
		}
	}

	if(tmp_username.size()==0
	   || tmp_password.size()==0
           || check_login(tmp_username,tmp_password)==-1)
	{
		throw Error(Error::AUTH);
	}
	set_header(new HTTPRedirectHeader(fmt.admin));
	set_login_cookies(tmp_username,tmp_password,365);
}

void Blog::logout()
{
	set_header(new HTTPRedirectHeader(global_config.sval("blog.script_path")));
	set_login_cookies("","",-1);
}

void Blog::admin()
{
	auth_or_throw();

	View_Admin view(this,c);
	view.ini_main();
	render.render(c,"admin",out.getstring());

}

void Blog::setup_blog()
{
	const vector<FormEntry> &form=cgi->getElements();


	View_Admin view(this,c);
	view.ini_share();

	string name,description,author,pass1,pass2;

	unsigned i;
	bool submit;

	for(i=0;i<form.size();i++) {
		string const &field=form[i].getName();
		if(field=="name") {
			name=form[i].getValue();
		}
		else if(field=="author") {
			author=form[i].getValue();
		}
		else if(field=="description") {
			description=form[i].getValue();
		}
		else if(field=="pass1") {
			pass1=form[i].getValue();
		}
		else if(field=="pass2") {
			pass2=form[i].getValue();
		}
	}

	submit=form.size()>0;

	row r;
	sql<<"SELECT count(*) FROM users",r;
	int n;
	r>>n;

	c["password_error"]=false;
	c["configured"]=false;
	c["field_error"]=false;
	c["blog_name"]=string("CppBlog");
	c["submit"]=fmt.admin;

	if(n!=0) {
		c["configured"]=true;
	}
	else if(submit) {
		if(pass1!=pass2) {
			c["password_error"]=true;
		}
		else if(name=="" || author=="" || description=="" || pass1=="") {
			c["field_error"]=true;
		}
		else {
			transaction tr(sql);
			sql<<	"INSERT INTO options(id,value) "
				"VALUES(?,?)",BLOG_TITLE,name,exec();
			sql<<	"INSERT INTO options(id,value) "
				"VALUES(?,?)",BLOG_DESCRIPTION,description,exec();
			sql<<	"INSERT INTO users(username,password) "
				"VALUES(?,?)",author,pass1,exec();
			sql<<	"INSERT INTO link_cats(name) VALUES('Links')",exec();
			int rowid=sql.rowid();
			sql<<	"INSERT INTO links(cat_id,title,url,description) "
				"VALUES (?,'CppCMS','http://cppcms.sourceforge.net/','') ",rowid,exec();
			tr.commit();
			c["configured"]=true;
		}
	}

	c["master_content"]=string("setup_blog");
	render.render(c,"admin",out.getstring());
}

void Blog::update_comment(string sid)
{
	auth_or_throw();

	int id=atoi(sid.c_str());

	const vector<FormEntry> &form=cgi->getElements();
	bool del=false;

	string author,url,email,content;

	unsigned i;
	for(i=0;i<form.size();i++) {
		string const &field=form[i].getName();
		if(field=="url") {
			url=form[i].getValue();
		}
		else if(field=="author") {
			author=form[i].getValue();
		}
		else if(field=="content") {
			content=form[i].getValue();
		}
		else if(field=="email") {
			email=form[i].getValue();
		}
		else if(field=="delete") {
			del=true;
		}
	}
	if(del) {
		del_single_comment(id);
	}
	else {
		sql<<	"UPDATE comments "
			"SET	url=?,author=?,content=?,email=? "
			"WHERE	id=?",
				url,author,content,email,id,exec();
	}
	set_header(new HTTPRedirectHeader(fmt.admin));
}

void Blog::edit_comment(string sid)
{
	auth_or_throw();
	int id=atoi(sid.c_str());

	View_Admin view(this,c);
	view.ini_cedit(id);
	render.render(c,"admin",out.getstring());

}
void Blog::edit_post(string sid,string ptype)
{
	auth_or_throw();

	int id= (sid == "new") ? -1 : atoi(sid.c_str()) ;

	View_Admin view(this,c);
	view.ini_edit(id,ptype);
	render.render(c,"admin",out.getstring());

}

void Blog::get_post(string sid,string ptype)
{
	auth_or_throw();

	unsigned i;
	int id=sid=="new" ? -1 : atoi(sid.c_str());
	const vector<FormEntry> &form=cgi->getElements();

	string title,abstract,content;

	enum { SAVE, PUBLISH, UNPUBLISH, PREVIEW, DELETE } type;
	type = SAVE;

	set<int> dellist;
	set<int> addlist;

	for(i=0;i<form.size();i++) {
		string const &field=form[i].getName();
		if(field=="title") {
			title=form[i].getValue();
		}
		else if(field=="abstract") {
			abstract=form[i].getValue();
		}
		else if(field=="content") {
			content=form[i].getValue();
		}
		else if(field=="publish") {
			type=PUBLISH;
		}
		else if(field=="unpublish") {
			type=UNPUBLISH;
		}
		else if(field=="save") {
			type=SAVE;
		}
		else if(field=="delete") {
			type=DELETE;
		}
		else if(field=="preview") {
			type=PREVIEW;
		}
		else if(field=="add") {
			addlist.insert(form[i].getIntegerValue());
		}
		else if(field=="del") {
			dellist.insert(form[i].getIntegerValue());
		}
	}

	if(id!=-1 && type==DELETE) {
		if(ptype=="post") {
			transaction tr(sql);
			sql<<"DELETE FROM post2cat WHERE post_id=?",id,exec();
			sql<<"DELETE FROM comments WHERE post_id=?",id,exec();
			sql<<"DELETE FROM posts WHERE id=?",id,exec();
			tr.commit();
		}
		else {
			sql<<"DELETE FROM pages WHERE id=?",id,exec();
		}
	}
	else {
		int p;
		if(type==PUBLISH)
			p=1;
		else if(type==UNPUBLISH)
			p=-1;
		else {
			p=0;
		}
		if(ptype=="post")
			save_post(id,title,abstract,content,p,addlist,dellist);
		else
			save_page(id,title,content,p);
	}
	if(type==SAVE || type==DELETE ){
		set_header(new HTTPRedirectHeader(fmt.admin));
	}
	else if(type==PREVIEW || type==UNPUBLISH) {
		edit_post(str(format("%1%") % id),ptype);
	}
	else /*(type==PUBLISH )*/{
		string redirect;
		if(ptype=="post")
			redirect=str(format(fmt.post)%id);
		else
			redirect=str(format(fmt.page)%id);

		set_header(new HTTPRedirectHeader(redirect));
	}
}

void Blog::save_page(int &id,string &title,
		     string &content,int pub)
{
	int is_open = pub ? 1: 0;

	if(id==-1) {
		sql<<	"INSERT INTO pages (author_id,title,content,is_open) "
			"VALUES(?,?,?,?)",
			userid,title,content,is_open;
		sql.exec();
		id=sql.rowid("page_id_seq");
		// The parameter is relevant for PgSQL only
	}
	else {
		if(pub){
			sql<<	"UPDATE pages "
				"SET	title= ?,"
				"	content=?,"
				"	is_open=? "
				"WHERE id=?",
			title,content,(pub>0?1:0),id;
			sql.exec();
		}
		else {
			sql<<	"UPDATE pages "
				"SET	title= ?,"
				"	content=? "
				"WHERE id=?",
			title,content,
			id;
			sql.exec();
		}
	}
}

void Blog::save_post(int &id,string &title,
		     string &abstract,string &content,int pub,set<int> &addlist,set<int> &dellist)
{
	tm t;
	time_t tt=time(NULL);

	localtime_r(&tt,&t);
	int is_open = pub ? 1: 0;

	transaction tr(sql);

	if(id==-1) {
		sql<<	"INSERT INTO posts (author_id,title,abstract,content,is_open,publish) "
			"VALUES(?,?,?,?,?,?)",
			userid,title,abstract,
			content,is_open,t;
		sql.exec();
		id=sql.rowid("posts_id_seq");
		// The parameter is relevant for PgSQL only
	}
	else {
		if(pub==0)	{
			sql<<	"UPDATE posts "
				"SET	title= ?,"
				"	abstract= ?,"
				"	content=?"
				"WHERE id=?",
			title,abstract,content,id;
			sql.exec();
			row r;
			sql<<	"SELECT publish,is_open FROM posts WHERE id=?",id;
			if(sql.single(r)) {
				r>>t>>is_open;
			}
		}
		else if(pub>0)	{
			sql<<	"UPDATE posts "
				"SET	title=?,abstract=?,content=?,is_open=1,publish=? "
				"WHERE id=?",
				title,abstract,content,t,id;
			sql.exec();
			sql<<	"UPDATE post2cat "
				"SET	publish=?,is_open=1 "
				"WHERE	post_id=?",t,id,exec();
		}
		else {
			sql<<	"UPDATE posts "
				"SET	title=?,abstract=?,content=?,is_open=0 "
				"WHERE id=?",
				title,abstract,content,id;
			sql.exec();
			sql<<	"UPDATE post2cat "
				"SET	is_open=0 "
				"WHERE	post_id=?",id,exec();
			row r;
			sql<<	"SELECT publish FROM posts WHERE id=?",id;
			if(sql.single(r)) {
				r>>t;
			}
		}
	}

	set<int>::iterator p;
	set<int> existing;
	result res;
	sql<<"SELECT id FROM cats",res;
	row r;
	while(res.next(r)) {
		int id;
		r>>id;
		existing.insert(id);
	}
	for(p=addlist.begin();p!=addlist.end();p++) {
		// Test if we not adding removed/not existing category
		if(existing.find(*p)==existing.end())
			continue;
		sql<<	"INSERT INTO post2cat(post_id,cat_id,is_open,publish) "
				"VALUES(?,?,?,?)",id,*p,is_open,t,exec();
	}
	for(p=dellist.begin();p!=dellist.end();p++) {
		sql<<	"DELETE FROM post2cat WHERE post_id=? AND cat_id=?",
		id,(*p),exec();
	}

	tr.commit();
}

void Blog::del_single_comment(int id)
{
	transaction tr(sql);
	row r;
	sql<<	"SELECT post_id FROM comments WHERE id=?",id;
	if(sql.single(r)) {
		int post_id;
		r>>post_id;
		sql<<	"DELETE FROM comments "
			"WHERE id=?",id,exec();
		count_comments(post_id);
	}
	tr.commit();
}

void Blog::del_comment(string sid)
{
	auth_or_throw();
	int id=atoi(sid.c_str());
	del_single_comment(id);
	set_header(new HTTPRedirectHeader(env->getReferrer()));
}


void Blog::feed(string cat)
{

	set_header(new HTTPContentHeader("text/xml"));

	View_Main_Page view(this,c);
	if(cat=="all")
		view.ini_main(-1,true);
	else
		view.ini_main(-1,true,atoi(cat.c_str()));
	render.render(c,"feed_posts",out.getstring());
}

void Blog::feed_comments()
{
	set_header(new HTTPContentHeader("text/xml"));

	View_Main_Page view(this,c);
	view.ini_rss_comments();
	render.render(c,"feed_comments",out.getstring());
}


