#include "blog.h"
#include "templates/look.h"
#include <cppcms/templates.h>

#include <time.h>
#include <boost/format.hpp>
#include <cgicc/HTTPRedirectHeader.h>
#include "error.h"

using namespace cgicc;
using namespace soci;
using boost::format;
using boost::str;

struct In_Comment {
	string message;
	string author;
	string url;
	string email;
	bool load(const vector<FormEntry> &form);
};

bool In_Comment::load(const vector<FormEntry> &form)
{
	int i;
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

	//url.add("^/admin/edit_comment/(\\d+)$",
	//	boost::bind(&Blog::edit_comment,this,$1));
	//fmt.edit_comment=root+"/adin/edit_comment/%1%";
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
		sql.open(global_config.sval("soci.engine"),global_config.sval("soci.connect"));
	}
	catch(soci_error const &e){
		throw HTTP_Error(string("Failed to access DB")+e.what());
	}
	connected=true;
}


void Blog::post(string s_id,bool preview)
{
	if(preview) {
		auth_or_throw();
	}
	int id=atoi(s_id.c_str());
	Content c(T_VAR_NUM);

	Renderer r(templates,TT_master,c);
	View_Main_Page view(this);
	view.ini_post(id,preview);
	view.render(r,c,out.getstring());
}

void Blog::main_page(string from)
{

	Content c(T_VAR_NUM);

	Renderer r(templates,TT_master,c);
	View_Main_Page view(this);
	if(from=="end") {
		view.ini_main(-1);
	}
	else {
		view.ini_main(atoi(from.c_str()));
	}
	view.render(r,c,out.getstring());
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
	catch (soci_error &err) {
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

	sql<<"SELECT is_open FROM posts WHERE id=:id",
		use(post_id),into(post.is_open);

	if(!post.is_open) {
		throw Error(Error::E404);
	}

	tm t;
	time_t cur=time(NULL);
	localtime_r(&cur,&t);

	sql<<	"INSERT INTO "
		"comments (post_id,author,url,email,publish_time,content) "
		"values(:post_id,:author,:url,:email,:tm,:content)",
		use(post_id),use(incom.author),use(incom.url),
		use(incom.email),use(t),use(incom.message);

	string redirect=str(format(fmt.post) % post_id);
	set_header(new HTTPRedirectHeader(redirect));
}

void Blog::error_page(int what)
{
	Content c(T_VAR_NUM);

	if(what==Error::AUTH) {
		Renderer r(templates,TT_admin,c);
		View_Admin view(this);
 		view.ini_login();
		view.render(r,c,out.getstring());
	}
	else {
		Renderer r(templates,TT_master,c);
		View_Main_Page view(this);
		view.ini_error(what);
		view.render(r,c,out.getstring());
	}
}

int Blog::check_login( string username,string password)
{
	int id=-1;
	string pass;
	sql<<	"SELECT id,password FROM users WHERE username=:u",
		use(username),
		into(id),into(pass);
	if(id!=-1 && password==pass) {
		return id;
	}
	return -1;
}

bool Blog::auth()
{
	int id,i;
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
	int i;
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

	Content c(T_VAR_NUM);

	Renderer r(templates,TT_admin,c);
	View_Admin view(this);
	view.ini_main();
	view.render(r,c,out.getstring());

}

void Blog::edit_post(string sid)
{
	auth_or_throw();

	int id= (sid == "new") ? -1 : atoi(sid.c_str()) ;

	Content c(T_VAR_NUM);

	Renderer r(templates,TT_admin,c);
	View_Admin view(this);
	view.ini_edit(id);
	view.render(r,c,out.getstring());

}

void Blog::get_post(string sid)
{
	auth_or_throw();

	int i;
	int id=sid=="new" ? -1 : atoi(sid.c_str());
	const vector<FormEntry> &form=cgi->getElements();

	string title,abstract,content;

	enum { SAVE, PUBLISH, PREVIEW } type;

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
		int i;
		sql.begin();
		sql<<	"INSERT INTO posts (author_id,title,abstract,content,is_open,publish) "
			"VALUES(:author_id,:title,:abstract,:content,:is_open,:tm)",
			use(userid),use(title),use(abstract),
			use(content),use(is_open),use(t);
		sql<<	"SELECT id FROM posts ORDER BY id DESC LIMIT 1",into(id);
		sql.commit();
	}
	else {
		sql<<	"UPDATE posts "
			"SET	title= :title,"
			"	abstract= :abstract,"
			"	content=:content,"
			"	is_open=is_open OR :open,"
			"	publish=:tm "
			"WHERE id=:id",
			use(title),use(abstract),use(content),
			use(is_open),use(t),use(id);
	}
}

void Blog::del_comment(string sid)
{
	auth_or_throw();
	int id=atoi(sid.c_str());
	sql<<"DELETE FROM comments WHERE id=:id",use(id);
	set_header(new HTTPRedirectHeader(env->getReferrer()));
}


void Blog::feed()
{

	Content c(T_VAR_NUM);

	Renderer r(templates,TT_feed_posts,c);
	View_Main_Page view(this);
	view.ini_main(-1,true);
	view.render(r,c,out.getstring());
}


