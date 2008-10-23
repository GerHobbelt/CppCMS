//
// C++ Implementation: views
//
// Description:
//
//
// Author: artik <artik@art-laptop>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <cppcms/manager.h>
#include "views.h"
#include <boost/format.hpp>
#include "blog.h"
#include "error.h"
#include "cxxmarkdown/markdowncxx.h"
#include <set>

using boost::format;
using boost::str;

using namespace dbixx;


void View_Post::ini_share(data::post_data &c)
{
	if(blog->userid!=-1){
		c.edit_url=str(format(blog->fmt.edit_post) % c.id);
	}
	c.permlink=str(format(blog->fmt.post) % c.id);
}

void View_Post::ini_full(data::post &c)
{
	ini_share(c);
	
	c.subtitle=c.title;
	
	if(c.content!="") {
		c.has_content=true;
	}
	else {
		c.has_content=false;
	}

	// Simple ANTI SPAM protection
	string post_comment=str(format(blog->fmt.add_comment) % c.id);
	int n=post_comment.size()/2;
	string post_comment_url_2=post_comment.substr(n);
	string post_comment_url_1=post_comment.substr(0,n);
	c.post_comment_url_1=post_comment_url_1;
	c.post_comment_url_2=post_comment_url_2;

	string trackback=str(format(blog->fmt.trackback) % c.id);
	n=trackback.size()/2;
	string tb_2=trackback.substr(n);
	string tb_1=trackback.substr(0,n);
	c.trackback_part_1=tb_1;
	c.trackback_part_2=tb_2;

	result rs;
	blog->sql<<
		"SELECT id,author,url,publish_time,content "
		"FROM comments "
		"WHERE post_id=? "
		"ORDER BY publish_time",c.id;
	blog->sql.fetch(rs);

	row cur;

	c.comments.resize(rs.rows());

	int i;
	for(i=0;rs.next(cur);i++)
	{
		data::comment_data &com=c.comments[i];
		cur	>> com.id >>com.username >> com.url
			>> com.date >> com.content ;
		if(blog->userid!=-1){
			com.delete_url=str(format(blog->fmt.del_comment) % com.id);
			com.edit_url=str(format(blog->fmt.edit_comment) % com.id);
		}
	}

	blog->sql<<
		"SELECT post2cat.cat_id,cats.name "
		"FROM	post2cat "
		"JOIN	cats ON post2cat.cat_id=cats.id "
		"WHERE	post2cat.post_id=?",c.id,rs;
	for(i=0;rs.next(cur);i++) {
		data::category cat;
		int id;
		cur>>id>>cat.name;
		cat.url=str(format(blog->fmt.cat) % id);
		c.post_cats.push_back(cat);
	}
}


void View_Post::ini_short(data::post_data &p)
{
	ini_share(p);
}

void View_Main_Page::ini_sidebar(set<string> &triggers,data::sidebar &c)
{
	result res;
	row r;
	blog->sql<<
		"SELECT	value "
		"FROM	text_options "
		"WHERE	id='copyright'";
	if(blog->sql.single(r)){
		string val;
		r>>c.copyright_string;
	}

	triggers.insert("options");

	blog->sql<<
		"SELECT id,title "
		"FROM	pages "
		"WHERE	is_open=1",res;
	c.pages.resize(res.rows());
	int i;
	for(i=0;res.next(r);i++) {
		int id;
		r>>id>>c.pages[i].title;
		c.pages[i].link=str(boost::format(blog->fmt.page) % id);
		triggers.insert(str(boost::format("page_%1%") % id));
	}
	triggers.insert("pages");

	blog->sql<<
		"SELECT link_cats.id,name,title,url,description "
		"FROM link_cats,links "
		"WHERE links.cat_id=link_cats.id "
		"ORDER BY link_cats.id",res;

	list<data::sidebar::link_cat> &link_cats=c.link_cats;
	int previd=-1;
	list<data::sidebar::link_cat::link> *lptr=NULL;
	while(res.next(r)){
		int id;
		string gname,title,url,descr;
		r>>id>>gname>>title>>url>>descr;
		if(previd==-1 || id!=previd) {
			data::sidebar::link_cat content;
			content.title=gname;
			link_cats.push_back(content);
			lptr=&(link_cats.back().links);
			previd=id;
		}
		data::sidebar::link_cat::link ctmp;
		ctmp.href=url;
		ctmp.title=title;
		ctmp.description=descr;
		lptr->push_back(ctmp);
	}

	triggers.insert("links");

	blog->sql<<"SELECT id,name FROM	cats",res;
	c.cats.resize(res.rows());
	for(i=0;res.next(r);i++) {
		int id;
		string name;
		r>>id>>c.cats[i].name;
		c.cats[i].link=str(boost::format(blog->fmt.cat) % id);
	}
	triggers.insert("categories");
	c.markdown2html=boost::bind(&Blog::markdown,blog,_1);
}

struct blog_options : public serializable
{
	string name,description,contact;
	virtual void load(archive &a) { a>>name>>description>>contact; };
	virtual void save(archive &a) const { a<<name<<description<<contact; };
};

void View_Main_Page::ini_share(data::master &c)
{
	int id;
	string val;
	result rs;
	blog_options options;
	if(blog->cache.fetch_data("options",options)){
		c.blog_name=options.name;
		c.blog_description=options.description;
		c.blog_contact=options.contact;
	}
	else {
		blog->sql<<"SELECT id,value FROM options";
		blog->sql.fetch(rs);
		row i;
		for(;rs.next(i);){
			i >>id>>val;
			if(id==BLOG_TITLE)
				c.blog_name=options.name=val;
			else if(id==BLOG_DESCRIPTION)
				c.blog_description=options.description=val;
			else if(id==BLOG_CONTACT && val!="")
				c.blog_contact=options.contact=val;
		}
		blog->cache.store_data("options",options);
	}
	c.media=blog->fmt.media;
	c.admin_url=blog->fmt.admin;
	c.base_url=blog->app.config.sval("blog.script_path");
	c.host=blog->app.config.sval("blog.host");
	c.rss_posts=blog->fmt.feed;
	c.rss_comments=blog->fmt.feed_comments;
	c.cookie_prefix=blog->app.config.sval("blog.id","");

	c.on_sidebar_load=boost::bind(
			&View_Main_Page::on_sidebar_load,
			this,
			_1);
	c.markdown2html=boost::bind(&Blog::markdown,blog,_1);
	c.latex=boost::bind(&Blog::latex,blog,_1);

}

void View_Main_Page::on_sidebar_load(string &sidebar)
{
	if(blog->cache.fetch_frame("sidebar",sidebar)) {
		return;
	}
	
	ostringstream oss;
	
	data::sidebar c;
	set<string> triggers;
	ini_sidebar(triggers,c);
	blog->render("sidebar",c,oss);
	sidebar=oss.str();
	
	blog->cache.store_frame("sidebar",sidebar,triggers);
}

void View_Main_Page::ini_rss_comments(data::feed_comments &c)
{
	ini_share(c);
	result res;
	blog->sql<<
		"SELECT id,post_id,author,content "
		"FROM comments "
		"ORDER BY id DESC "
		"LIMIT 10",res;
	c.comments.resize(res.rows());
	row r;
	int i;
	for(i=0;res.next(r);i++){
		int pid;
		r>>c.comments[i].id>>pid>>c.comments[i].author>>c.comments[i].content;
		c.comments[i].permlink=str(boost::format(blog->fmt.post) % pid);
	}
}

void View_Main_Page::prepare_query(int cat_id,int id,int max_posts)
{
	if(cat_id==-1) {
		if(id!=-1) {
			blog->sql<<
				"SELECT posts.id,users.username,posts.title, "
				"	posts.abstract, posts.content !='', "
				"	posts.publish,posts.comment_count "
				"FROM	posts "
				"LEFT JOIN "
				"	users ON users.id=posts.author_id "
				"WHERE	posts.publish <= "
					"(SELECT publish FROM posts WHERE id=?) "
				"	AND posts.is_open=1 "
				"ORDER BY posts.publish DESC "
				"LIMIT ?",
				id,max_posts+1;
		}
		else {
			blog->sql<<
				"SELECT posts.id,users.username,posts.title, "
				"	posts.abstract, posts.content!='', "
				"	posts.publish,posts.comment_count "
				"FROM	posts "
				"LEFT JOIN "
				"	users ON users.id=posts.author_id "
				"WHERE	posts.is_open=1 "
				"ORDER BY posts.publish DESC "
				"LIMIT ?",(max_posts+1);
		}
	}
	else {
		if(id!=-1) {
			blog->sql<<
				"SELECT posts.id,users.username,posts.title, "
				"	posts.abstract, posts.content !='', "
				"	posts.publish,posts.comment_count "
				"FROM	post2cat "
				"LEFT JOIN	posts ON post2cat.post_id=posts.id "
				"LEFT JOIN	users ON users.id=posts.author_id "
				"WHERE	post2cat.cat_id=? "
				"	AND post2cat.is_open=1 "
				"	AND post2cat.publish <= "
				"		(SELECT publish FROM posts WHERE id=?) "
				"ORDER BY post2cat.publish DESC "
				"LIMIT ?",
				cat_id,id,max_posts;
		}
		else {
			blog->sql<<
				"SELECT posts.id,users.username,posts.title, "
				"	posts.abstract, posts.content !='', "
				"	posts.publish,posts.comment_count "
				"FROM	post2cat "
				"LEFT JOIN	posts ON post2cat.post_id=posts.id "
				"LEFT JOIN	users ON users.id=posts.author_id "
				"WHERE	post2cat.cat_id=? "
				"	AND post2cat.is_open=1 "
				"ORDER BY post2cat.publish DESC "
				"LIMIT ?",
				cat_id,max_posts;
		}

	}
}

void View_Main_Page::ini_main(int id,bool feed,int cat_id,data::main_page &c)
{
	ini_share(c);

	int max_posts=feed ? 10 : 5;

	c.posts.resize(max_posts);
	vector<data::post_data> &latest_posts=c.posts;

	result rs;

	prepare_query(cat_id,id,max_posts+1);

	blog->sql.fetch(rs);

	row r;
	int counter,n;

	std::tm first,last;

	std::map<int,list<data::category> *> post2cat_list;

	for(counter=1,n=0;rs.next(r);counter++) {
		if(counter==max_posts+1) {
			int id;
			r>>id;
			if(cat_id==-1)
				c.next_page_link=str(format(blog->fmt.main_from) % id);
			else
				c.next_page_link=str(format(blog->fmt.cat_from) % cat_id % id);
			break;
		}
		
		data::post_data &post=latest_posts[n];

		r>>post.id;
		r>>post.author;
		r>>post.title;
		r>>post.abstract;
		r>>post.has_content;
		r>>post.date;
		r>>post.comment_count;

		if(!feed){
			post2cat_list[post.id]=&post.post_cats;
		}

		blog->cache.add_trigger(str(boost::format("post_%1%") % post.id));
		if(!feed)
			blog->cache.add_trigger(str(boost::format("comments_%1%") % post.id));

		if(counter==1)
			first=post.date;
		last=post.date;

		View_Post post_v(blog);
		post_v.ini_short(post);
		n++;
	}

	string cat_name;

	if(n>0 && !feed) {
		if(cat_id==-1){
			blog->sql<<
			"SELECT	list.pid,cats.id,cats.name "
			"FROM	(	SELECT posts.id as pid"
			"		FROM posts "
			"		WHERE publish>=? and publish<=? and is_open=1)"
			"	AS list"
			"	JOIN post2cat ON list.pid=post2cat.post_id"
			"	JOIN cats on post2cat.cat_id=cats.id",last,first;
		}
		else {
			blog->sql<<
			"SELECT	list.pid,cats.id,cats.name "
			"FROM	(SELECT post2cat.post_id as pid "
			"	FROM post2cat "
			"	WHERE publish>=? and publish<=? and is_open=1 and cat_id=?)"
			"	AS list"
			"	JOIN post2cat ON list.pid=post2cat.post_id"
			"	JOIN cats on post2cat.cat_id=cats.id",last,first,cat_id;
		}
		result res;
		row r;
		blog->sql.fetch(res);
		while(res.next(r)) {
			int cid,pid;
			string name;
			r>>pid>>cid>>name;
			if(cid==cat_id && cat_name==""){
				cat_name=name;
			}
			map<int,list<data::category> *>::iterator ptr;
			if((ptr=post2cat_list.find(pid))!=post2cat_list.end()) {
				list<data::category> &temp=(*(ptr->second));
				temp.push_back(data::category());
				temp.back().name=name;
				temp.back().url=str(format(blog->fmt.cat) % cid);
			}
		}
	}

	if(cat_id!=-1) {
		row r;
		if(cat_name==""){
			blog->sql<<"SELECT name FROM cats WHERE id=?",cat_id;
			if(!blog->sql.single(r))
				throw Error(Error::E404);
			r>>cat_name;
		}
		c.category_name=cat_name;
		c.category_rss=str(format(blog->fmt.feed_cats) % cat_id);
		c.subtitle=cat_name;
	}
	
	if(n<max_posts){
		latest_posts.resize(n);
	}
}


void View_Main_Page::ini_page(int id,bool preview,data::page &c)
{
	ini_share(c);
	row r;
	blog->sql<<
		"SELECT title,content,is_open "
		"FROM	pages "
		"WHERE	id=?",id;
	if(!blog->sql.single(r))
		throw Error(Error::E404);
	int is_open;
	r >>c.title>>c.content>>is_open;
	c.subtitle=c.title;
	if(!is_open && !preview) throw Error(Error::E404);
}

void View_Main_Page::ini_post(int id,bool preview,data::post &c)
{
	View_Post post_v(blog);
	c.id=-1;
	row r;
	blog->sql<<
		"SELECT posts.id,users.username,posts.title, "
		"	posts.abstract, posts.content, "
		"	posts.publish,posts.is_open,posts.comment_count "
		"FROM	posts "
		"JOIN	users ON users.id=posts.author_id "
		"WHERE	posts.id=? ",
		id;
	if(!blog->sql.single(r)) {
		throw Error(Error::E404);
	}
	r>>	c.id>>c.author>>c.title>>
		c.abstract>>c.content>>
		c.date>>c.is_open>>c.comment_count;

	if(c.id==-1 || (!c.is_open && !preview)){
		throw Error(Error::E404);
	}

	ini_share(c);

	post_v.ini_full(c);
}

void View_Main_Page::ini_error(int id,data::error &c)
{
	ini_share(c);
	switch(id){
	case Error::E404:
		c.error_404=true;
		c.error_comment=false;
		break;
	case Error::COMMENT_FIELDS:
		c.error_404=false;
		c.error_comment=true;
		break;
	}
}

void View_Admin::ini_share(data::admin_base &c)
{
	row r;
	blog->sql<<
		"SELECT value FROM options WHERE id=?",
		(int)BLOG_TITLE;
	if(blog->sql.single(r))
		r>>c.blog_name;
	c.media=blog->fmt.media;
	c.base_url=blog->app.config.sval("blog.script_path");
	c.admin_url=blog->fmt.admin;
	c.logout_url=blog->fmt.logout;
	c.new_post_url=blog->fmt.new_post;
	c.new_page_url=blog->fmt.new_page;
	c.edit_options_url=blog->fmt.edit_options;
	c.edit_links_url=blog->fmt.edit_links;
	c.edit_cats_url=blog->fmt.edit_cats;
	c.admin_cache_url=blog->fmt.admin_cache;
	c.cookie_prefix=blog->app.config.sval("blog.id","");
}

void View_Admin::ini_options(data::admin_editoptions &c)
{
	ini_share(c);
	result res;
	blog->sql<<"SELECT id,value FROM options",res;
	row r;
	while(res.next(r)) {
		int id;
		string val;
		r>>id>>val;
		if(id==BLOG_TITLE)
			c.blog_name=val;
		else if(id==BLOG_DESCRIPTION)
			c.blog_description=val;
		else if(id==BLOG_CONTACT)
			c.blog_contact=val;
	}
	blog->sql<<"SELECT value FROM text_options WHERE id='copyright'";
	if(blog->sql.single(r)) {
		string val;
		r>>c.copyright_string;
	}
	c.post_options_url=blog->fmt.edit_options;
}

void View_Admin::ini_login(data::admin_login &c)
{
	ini_share(c);
	c.login_url=blog->fmt.login;
}

void View_Admin::ini_cedit(int id,data::admin_editcomment &c)
{
	ini_share(c);

	c.edit_comment_url=str(format(blog->fmt.update_comment) % id);
	c.id=id;
	string author,url,content,email;
	blog->sql<<
		"SELECT author,url,email,content "
		"FROM	comments "
		"WHERE	id=?",id;
	row r;
	if(!blog->sql.single(r)) {
		throw Error(Error::E404);
	}
	r>>c.author>>c.url>>c.email>>c.content;
}

void View_Admin::ini_editpage(int id,data::admin_editpage &c)
{
	ini_share(c);

	c.post_id=id;

	if(id!=-1) {
		c.post_id=id;
		c.submit_post_url=str(format( blog->fmt.update_page) % id);
		c.preview_url=str(format(blog->fmt.page_preview) % id);
	}
	else {
		c.submit_post_url=blog->fmt.add_page;
		c.is_open=0;
		return;
	}
	row r;
	blog->sql<<
		"SELECT id,title,content,is_open "
		"FROM pages "
		"WHERE id=?",
		id;
	if(!blog->sql.single(r)){
		throw Error(Error::E404);
	}
	r>>c.post_id>>c.post_title>>c.content>>c.is_open;
}

void View_Admin::ini_editpost(int id,data::admin_editpost &c)
{
	ini_share(c);
	
	c.post_id=id;
	set<int> inset;
	result res;
	row r;
	if(id!=-1) {
		blog->sql<<
			"SELECT cats.id,cats.name "
			"FROM	post2cat "
			"JOIN	cats ON post2cat.cat_id=cats.id "
			"WHERE	post2cat.post_id=?",id;
		blog->sql.fetch(res);
		c.cats_in.resize(res.rows());
		int i;
		for(i=0;res.next(r);i++) {
			int cid;
			r>>cid>>c.cats_in[i].name;
			inset.insert(cid);
			c.cats_in[i].id=cid;
		}
	}

	blog->sql<<"SELECT id,name FROM cats",res;
	while(res.next(r)) {
		data::category ctmp;
		r>>ctmp.id>>ctmp.name;
		if(inset.find(ctmp.id)==inset.end()) {
			c.cats_out.push_back(ctmp);
		}
	}

	if(id!=-1) {
		c.submit_post_url=str(format( blog->fmt.update_post) % id);
		c.preview_url=str(format(blog->fmt.preview) % id);
		c.send_trackback_url=blog->fmt.send_trackback;
	}
	else {
		c.submit_post_url=blog->fmt.add_post;
		c.is_open=0;
		return;
	}

	blog->sql<<
		"SELECT title,abstract,content,is_open "
		"FROM posts "
		"WHERE id=?",
		id;
	if(!blog->sql.single(r)){
		throw Error(Error::E404);
	}
	r>>	c.post_title>>
		c.abstract>>c.content>>c.is_open;
	
}

void View_Admin::ini_main(data::admin_main &c)
{
	ini_share(c);
	result rs;

	blog->sql<<
		"SELECT id,title "
		"FROM posts "
		"WHERE is_open=0";
	blog->sql.fetch(rs);
	row r;

	c.posts.resize(rs.rows());

	int i;
	for(i=0;rs.next(r);i++) {
		int id;
		string intitle;
		r>>id>>c.posts[i].title;
		string edit_url=str(format(blog->fmt.edit_post) % id);
		c.posts[i].edit_url=edit_url;
	}

	blog->sql<<
		"SELECT id,title,is_open "
		"FROM	pages ";
	blog->sql.fetch(rs);

	c.pages.resize(rs.rows());

	for(i=0;rs.next(r);i++) {
		int id;
		r>>id>>c.pages[i].title>>c.pages[i].published;
		string edit_url=str(format(blog->fmt.edit_page) % id);
		c.pages[i].edit_url=edit_url;
	}
	blog->sql<<
		"SELECT id,post_id,author "
		"FROM comments "
		"ORDER BY id DESC "
		"LIMIT 10",rs;

	c.comments.resize(rs.rows());

	for(i=0;rs.next(r);i++) {
		int id,c_id;
		string author;
		r>>c_id>>id>>c.comments[i].username;

		c.comments[i].post_permlink=str(format(blog->fmt.post) % id);
		c.comments[i].edit_url=str(format(blog->fmt.edit_comment) % c_id );
		c.comments[i].id=c_id;
	}
}

void View_Admin::ini_links(data::admin_editlinks &c)
{
	ini_share(c);
	result res;
	row r;
	blog->sql<<
		"SELECT id,name FROM link_cats",res;
	c.link_cats.resize(res.rows());
	int i;
	for(i=0;res.next(r);i++) {
		string name;
		r>>c.link_cats[i].id>>c.link_cats[i].name;
		c.link_cats[i].del=false;
	}

	c.submit_url=blog->fmt.edit_links;

	blog->sql<<
		"SELECT id,cat_id,title,url,description "
		"FROM links "
		"ORDER BY cat_id",res;
	c.links.resize(res.rows());

	for(i=0;res.next(r);i++) {
		string title,url,description;
		r>>	c.links[i].id>>
			c.links[i].cat_id>>
			c.links[i].name>>
			c.links[i].url>>
			c.links[i].descr;
	}
}


void View_Admin::ini_cats(data::admin_editcats &c)
{
	ini_share(c);
	result res;
	row r;
	blog->sql<<"SELECT id,name FROM cats",res;
	c.cats.resize(res.rows());
	int i;
	for(i=0;res.next(r);i++) {
		string name;
		r>>c.cats[i].id>>c.cats[i].name;
	}

	c.submit_url=blog->fmt.edit_cats;
}
