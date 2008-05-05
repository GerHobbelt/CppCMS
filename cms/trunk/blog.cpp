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

	url.add("^/?$",
		boost::bind(&Blog::main_page,this,"end"));
	fmt.main=root + "/";
	url.add("^/from/(\\d+)$",
		boost::bind(&Blog::main_page,this,$1));
	fmt.main_from=root + "/from/%1%";
	url.add("^/post/(\\d+)$",
		boost::bind(&Blog::post,this,$1,false));
	fmt.post=root + "/post/%1%";
	url.add("^/post/preview/(\\d+)$",
		boost::bind(&Blog::post,this,$1,true));
	fmt.preview=root + "/post/preview/%1%";

	url.add("^/admin$",
		boost::bind(&Blog::admin,this));
	fmt.admin=root + "/admin";

	url.add("^/admin/new_post$",
		boost::bind(&Blog::edit_post,this,"new"));
	fmt.new_post=root + "/admin/new_post";

	url.add("^/admin/edit_post/(\\d+)$",
		boost::bind(&Blog::edit_post,this,$1));
	fmt.edit_post=root+"/admin/edit_post/%1%";

	url.add("^/admin/edit_comment/(\\d+)$",
		boost::bind(&Blog::edit_comment,this,$1));
	fmt.edit_comment=root+"/admin/edit_comment/%1%";

	// All incoming information

	url.add("^/postback/comment/(\\d+)$",
		boost::bind(&Blog::add_comment,this,$1));
	fmt.add_comment=root+"/postback/comment/%1%";

	url.add("^/postback/post/new$",
		boost::bind(&Blog::get_post,this,"new"));
	fmt.add_post=root+"/postback/post/new";

	url.add("^/postback/post/(\\d+)$",
		boost::bind(&Blog::get_post,this,$1));
	fmt.update_post=root+"/postback/post/%1%";

	url.add("^/postback/update_comment/(\\d+)$",
		boost::bind(&Blog::update_comment,this,$1));
	fmt.update_comment=root+"/postback/update_comment/%1%";

	//url.add("^/postback/approve$",
	//	boost::bind(&Blog::approve,this));
	fmt.approve=root+"/postback/approve";

	url.add("^/admin/login$",
		boost::bind(&Blog::login,this));
	fmt.login=root+"/admin/login";

	url.add("^/admin/logout$",
		boost::bind(&Blog::logout,this));
	fmt.logout=root+"/admin/logout";

	url.add("^/postback/delete/comment/(\\d+)$",
		boost::bind(&Blog::del_comment,this,$1));
	fmt.del_comment=
		root+"/postback/delete/comment/%1%";
	url.add("^/rss$",boost::bind(&Blog::feed,this));
	fmt.feed=root+"/rss";

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


void Blog::post(string s_id,bool preview)
{
	if(preview) {
		auth_or_throw();
	}
	int id=atoi(s_id.c_str());
	content c;

	View_Main_Page view(this,c);
	view.ini_post(id,preview);

	render.render(c,"master",out.getstring(),tr);
}

void Blog::main_page(string from)
{

	content c;

	View_Main_Page view(this,c);
	if(from=="end") {
		view.ini_main(-1);
	}
	else {
		view.ini_main(atoi(from.c_str()));
	}
	render.render(c,"master",out.getstring(),tr);
}

void Blog::date(tm t,string &d)
{
	char buf[80];
	snprintf(buf,80,"%02d/%02d/%04d, %02d:%02d",
		 t.tm_mday,t.tm_mon+1,t.tm_year+1900,
		 t.tm_hour,t.tm_min);
	d=buf;
}

void Blog::main()
{
	try {
		try{
			if(!connected)
				sql.reconnect();
			auth();
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
		throw HTTP_Error(err.what());
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

	row r;
	sql<<"SELECT is_open FROM posts WHERE id=?",
		post_id,r;
	r>>post.is_open;

	if(!post.is_open) {
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

	string redirect=str(format(fmt.post) % post_id);
	set_header(new HTTPRedirectHeader(redirect));
}

void Blog::error_page(int what)
{
	content c;

	if(what==Error::AUTH) {
		View_Admin view(this,c);
 		view.ini_login();
		render.render(c,"admin",out.getstring(),tr);
	}
	else {
		View_Main_Page view(this,c);
		view.ini_error(what);
		render.render(c,"master",out.getstring(),tr);
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

	content c;

	View_Admin view(this,c);
	view.ini_main();
	render.render(c,"admin",out.getstring(),tr);

}

void Blog::update_comment(string sid)
{
	auth_or_throw();

	int id=atoi(sid.c_str());

	const vector<FormEntry> &form=cgi->getElements();
	bool del=false;

	string author,url,email,content;

	int i;
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
		sql<<	"DELETE FROM comments "
			"WHERE id=?",id,exec();
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
	content c;

	View_Admin view(this,c);
	view.ini_cedit(id);
	render.render(c,"admin",out.getstring(),tr);

}
void Blog::edit_post(string sid)
{
	auth_or_throw();

	int id= (sid == "new") ? -1 : atoi(sid.c_str()) ;

	content c;

	View_Admin view(this,c);
	view.ini_edit(id);
	render.render(c,"admin",out.getstring(),tr);

}

void Blog::get_post(string sid)
{
	auth_or_throw();

	unsigned i;
	int id=sid=="new" ? -1 : atoi(sid.c_str());
	const vector<FormEntry> &form=cgi->getElements();

	string title,abstract,content;

	enum { SAVE, PUBLISH, PREVIEW } type;
	type = SAVE;

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
		else if(field=="save") {
			type=SAVE;
		}
		else if(field=="preview") {
			type=PREVIEW;
		}
	}

	save_post(id,title,abstract,content,type==PUBLISH);

	if(type==SAVE){
		set_header(new HTTPRedirectHeader(fmt.admin));
	}
	else if(type==PUBLISH){
		string redirect=str(format(fmt.post)%id);
		set_header(new HTTPRedirectHeader(redirect));
	}
	else if(type==PREVIEW) {
		edit_post(str(format("%1%") % id));
	}
}

void Blog::save_post(int &id,string &title,
		     string &abstract,string &content,bool pub)
{
	tm t;
	time_t tt=time(NULL);

	localtime_r(&tt,&t);
	int is_open = pub ? 1: 0;

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
		if(pub)	{
			sql<<	"UPDATE posts "
				"SET	title= ?,"
				"	abstract= ?,"
				"	content=?,"
				"	is_open=1,"
				"	publish=? "
				"WHERE id=?",
			title,abstract,content,
			t,id;
			sql.exec();
		}
		else {
			sql<<	"UPDATE posts "
				"SET	title= ?,"
				"	abstract= ?,"
				"	content=? "
				"WHERE id=?",
			title,abstract,content,
			id;
			sql.exec();
		}
	}
}

void Blog::del_comment(string sid)
{
	auth_or_throw();
	int id=atoi(sid.c_str());
	sql<<"DELETE FROM comments WHERE id=?",id;
	sql.exec();
	set_header(new HTTPRedirectHeader(env->getReferrer()));
}


void Blog::feed()
{

	content c;

	set_header(new HTTPContentHeader("text/xml"));

	View_Main_Page view(this,c);
	view.ini_main(-1,true);
	render.render(c,"feed_posts",out.getstring(),tr);
}


