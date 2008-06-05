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
#include "views.h"
#include <boost/format.hpp>
#include "blog.h"
#include "error.h"
#include "cxxmarkdown/markdowncxx.h"
#include <set>
#include <cppcms/global_config.h>

using boost::format;
using boost::str;

using namespace dbixx;

void View_Comment::init(comment_t &com)
{
	c["username"]=com.author;
	if(com.url.size()!=0){
		c["url"]=com.url;
	}
	c["content"]=com.content;
	c["date"]=com.publish_time;
	if(blog->userid!=-1){
		c["delete_url"]=str(format(blog->fmt.del_comment) % com.id);
		c["edit_url"]=str(format(blog->fmt.edit_comment) % com.id);
	}
}

void View_Post::ini_share(post_t &p)
{
	c["title"]=p.title;
	c["subtitle"]=p.title;
	c["date"]=p.publish;
	c["author"]=p.author_name;

	if(blog->userid!=-1){
		c["edit_url"]=str(format(blog->fmt.edit_post) % p.id);
	}
	c["permlink"]=str(format(blog->fmt.post) % p.id);
	c["comment_count"]=p.comment_count;
}

void View_Post::ini_full(post_t &p)
{
	ini_share(p);
	c["abstract"]=p.abstract;
	if(p.content!="") {
		c["content"]=p.content;
		c["has_content"]=true;
	}
	else {
		c["has_content"]=false;
	}

	// Simple ANTI SPAM protection
	string post_comment=str(format(blog->fmt.add_comment) % p.id);
	int n=post_comment.size()/2;
	string post_comment_url_2=post_comment.substr(n);
	string post_comment_url_1=post_comment.substr(0,n);
	c["post_comment_url_1"]=post_comment_url_1;
	c["post_comment_url_2"]=post_comment_url_2;

	string trackback=str(format(blog->fmt.trackback) % p.id);
	n=trackback.size()/2;
	string tb_2=trackback.substr(n);
	string tb_1=trackback.substr(0,n);
	c["trackback_part_1"]=tb_1;
	c["trackback_part_2"]=tb_2;

	result rs;
	blog->sql<<
		"SELECT id,author,email,url,publish_time,content "
		"FROM comments "
		"WHERE post_id=? "
		"ORDER BY publish_time",p.id;
	blog->sql.fetch(rs);

	row cur;

	content::vector_t &comments_list=c.vector("comments",rs.rows());

	int i;
	for(i=0;rs.next(cur);i++)
	{
		View_Comment com(blog,comments_list[i]);

		comment_t comment;
		cur	>> comment.id >>comment.author >> comment.email >> comment.url
			>> comment.publish_time >> comment.content ;

		com.init(comment);
	}

	blog->sql<<
		"SELECT post2cat.cat_id,cats.name "
		"FROM	post2cat "
		"JOIN	cats ON post2cat.cat_id=cats.id "
		"WHERE	post2cat.post_id=?",p.id,rs;
	content::vector_t &cats=c.vector("post_cats",rs.rows());
	for(i=0;rs.next(cur);i++) {
		int id;
		string name;
		cur>>id>>name;
		cats[i]["url"]=str(format(blog->fmt.cat) % id);
		cats[i]["name"]=name;
	}
}


void View_Post::ini_short(post_t &p)
{
	ini_share(p);
	c["abstract"]=p.abstract;
	c["has_content"]=(bool)p.has_content;
}

void View_Main_Page::ini_sidebar(set<string> &triggers,content &c)
{
	result res;
	row r;
	blog->sql<<
		"SELECT	value "
		"FROM	text_options "
		"WHERE	id='copyright'";
	if(blog->sql.single(r)){
		string val;
		r>>val;
		c["copyright_string"]=val;
	}

	triggers.insert("options");

	blog->sql<<
		"SELECT id,title "
		"FROM	pages "
		"WHERE	is_open=1",res;
	content::vector_t &pages=c.vector("pages",res.rows());
	int i;
	for(i=0;res.next(r);i++) {
		int id;
		string title;
		r>>id>>title;
		pages[i]["title"]=title;
		pages[i]["link"]=str(boost::format(blog->fmt.page) % id);
		triggers.insert(str(boost::format("page_%1%") % id));
	}
	triggers.insert("pages");

	blog->sql<<
		"SELECT link_cats.id,name,title,url,description "
		"FROM link_cats,links "
		"WHERE links.cat_id=link_cats.id "
		"ORDER BY link_cats.id",res;

	content::list_t &link_cats=c.list("link_cats");
	int previd=-1;
	content::list_t *lptr=NULL;
	while(res.next(r)){
		int id;
		string gname,title,url,descr;
		r>>id>>gname>>title>>url>>descr;
		if(previd==-1 || id!=previd) {
			link_cats.push_back(content());
			link_cats.back()["title"]=gname;
			lptr=&(link_cats.back().list("links"));
			previd=id;
		}
		lptr->push_back(content());
		content &ctmp=lptr->back();
		ctmp["href"]=url;
		ctmp["title"]=title;
		if(descr!="")
			ctmp["description"]=descr;
	}

	triggers.insert("links");

	blog->sql<<"SELECT id,name FROM	cats",res;
	content::vector_t &cats=c.vector("cats",res.rows());
	for(i=0;res.next(r);i++) {
		int id;
		string name;
		r>>id>>name;
		cats[i]["link"]=str(boost::format(blog->fmt.cat) % id);
		cats[i]["name"]=name;
	}
	triggers.insert("categories");

}

struct blog_options : public serializable 
{
	string name,description,contact;
	virtual void load(archive &a) { a>>name>>description>>contact; };
	virtual void save(archive &a) const { a<<name<<description<<contact; };
};

void View_Main_Page::ini_share()
{
	int id;
	string val;
	result rs;
	blog_options options;
	if(blog->cache.fetch_data("options",options)){
		c["blog_name"]=options.name;
		c["blog_description"]=options.description;
		c["blog_contact"]=options.contact;
	}
	else {
		blog->sql<<"SELECT id,value FROM options";
		blog->sql.fetch(rs);
		row i;
		for(;rs.next(i);){
			i >>id>>val;
			if(id==BLOG_TITLE)
				c["blog_name"]=options.name=val;
			else if(id==BLOG_DESCRIPTION)
				c["blog_description"]=options.description=val;
			else if(id==BLOG_CONTACT && val!="")
				c["blog_contact"]=options.contact=val;
		}
		blog->cache.store_data("options",options);
	}
	c["media"]=blog->fmt.media;
	c["admin_url"]=blog->fmt.admin;
	c["base_url"]=global_config.sval("blog.script_path");
	c["host"]=global_config.sval("blog.host");
	c["rss_posts"]=blog->fmt.feed;
	c["rss_comments"]=blog->fmt.feed_comments;
	c["cookie_prefix"]=global_config.sval("blog.id","");

	string sidebar;
	if(blog->cache.fetch_frame("sidebar",sidebar)) {
		c["sidebar"]=sidebar;
	}
	else {
		sidebar.reserve(16000);
		content sbar_content;
		set<string> triggers;
		ini_sidebar(triggers,sbar_content);
		blog->render.render(sbar_content,"sidebar",sidebar);
		c["sidebar"]=sidebar;
		blog->cache.store_frame("sidebar",sidebar,triggers);
	}
}

void View_Main_Page::ini_rss_comments()
{
	ini_share();
	result res;
	blog->sql<<
		"SELECT id,post_id,author,content "
		"FROM comments "
		"ORDER BY id DESC "
		"LIMIT 10",res;
	content::vector_t &comments=c.vector("comments",res.rows());
	row r;
	int i;
	for(i=0;res.next(r);i++){
		int id,pid;
		string author,content;
		r>>id>>pid>>author>>content;
		comments[i]["permlink"]=str(boost::format(blog->fmt.post) % pid);
		comments[i]["author"]=author;
		comments[i]["content"]=content;
		comments[i]["id"]=id;
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

void View_Main_Page::ini_main(int id,bool feed,int cat_id)
{
	ini_share();

	int max_posts=feed ? 10 : 5;

	content::vector_t &latest_posts=c.vector("posts",max_posts);

	result rs;

	prepare_query(cat_id,id,max_posts+1);

	blog->sql.fetch(rs);

	row r;
	int counter,n;

	std::tm first,last;

	std::map<int,content::list_t *> post2cat_list;

	for(counter=1,n=0;rs.next(r);counter++) {
		if(counter==max_posts+1) {
			int id;
			r>>id;
			if(cat_id==-1)
				c["next_page_link"]=str(format(blog->fmt.main_from) % id);
			else
				c["next_page_link"]=str(format(blog->fmt.cat_from) % cat_id % id);
			break;
		}
		post_t post;

		r>>post.id;
		r>>post.author_name;
		r>>post.title;
		r>>post.abstract;
		r>>post.has_content;
		r>>post.publish;
		r>>post.comment_count;

		if(!feed){
			post2cat_list[post.id]=&(latest_posts[n].list("post_cats"));
		}

		blog->cache.add_trigger(str(boost::format("post_%1%") % post.id));
		if(!feed)
			blog->cache.add_trigger(str(boost::format("comments_%1%") % post.id));

		if(counter==1)
			first=post.publish;
		last=post.publish;

		View_Post post_v(blog,latest_posts[n]);
		post_v.ini_short(post);
		n++;
	}
	if(n<max_posts){
		latest_posts.resize(n);
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
			map<int,content::list_t *>::iterator ptr;
			if((ptr=post2cat_list.find(pid))!=post2cat_list.end()) {
				content::list_t &temp=(*(ptr->second));
				temp.push_back(content());
				temp.back()["name"]=name;
				temp.back()["url"]=str(format(blog->fmt.cat) % cid);
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
		c["category_name"]=cat_name;
		c["category_rss"]=str(format(blog->fmt.feed_cats) % cat_id);
		c["subtitle"]=cat_name;
	}

	c["master_content"]=string("main_page");
}


void View_Main_Page::ini_page(int id,bool preview)
{
	ini_share();
	c["master_content"]=string("page");
	row r;
	blog->sql<<
		"SELECT title,content,is_open "
		"FROM	pages "
		"WHERE	id=?",id;
	if(!blog->sql.single(r))
		throw Error(Error::E404);
	string title;
	string content;
	int is_open;
	r >> title>>content>>is_open;
	if(!is_open && !preview) throw Error(Error::E404);
	c["content"]=content;
	c["title"]=title;
}

void View_Main_Page::ini_post(int id,bool preview)
{
	View_Post post_v(blog,c);
	post_t post;
	post.id=-1;
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
	r>>	post.id>>post.author_name>>post.title>>
		post.abstract>>post.content>>
		post.publish>>post.is_open>>post.comment_count;

	if(post.id==-1 || (!post.is_open && !preview)){
		throw Error(Error::E404);
	}

	ini_share();

	post_v.ini_full(post);
	c["master_content"]=string("post");
}

void View_Main_Page::ini_error(int id)
{
	ini_share();
	c["master_content"]=string("error");
	switch(id){
	case Error::E404:
		c["error_404"]=true;
		c["error_comment"]=false;
		break;
	case Error::COMMENT_FIELDS:
		c["error_404"]=false;
		c["error_comment"]=true;
		break;
	}
}

void View_Admin::ini_share()
{
	row r;
	blog->sql<<
		"SELECT value FROM options WHERE id=?",
		(int)BLOG_TITLE;
	string blog_name;
	if(blog->sql.single(r))
		r>>blog_name;
	c["blog_name"]=blog_name;
	c["media"]=blog->fmt.media;
	c["base_url"]=global_config.sval("blog.script_path");
	c["admin_url"]=blog->fmt.admin;
	c["logout_url"]=blog->fmt.logout;
	c["new_post_url"]=blog->fmt.new_post;
	c["new_page_url"]=blog->fmt.new_page;
	c["edit_options_url"]=blog->fmt.edit_options;
	c["edit_links_url"]=blog->fmt.edit_links;
	c["edit_cats_url"]=blog->fmt.edit_cats;
	c["admin_cache_url"]=blog->fmt.admin_cache;
	c["cookie_prefix"]=global_config.sval("blog.id","");
}

void View_Admin::ini_options()
{
	ini_share();
	result res;
	blog->sql<<"SELECT id,value FROM options",res;
	row r;
	while(res.next(r)) {
		int id;
		string val;
		r>>id>>val;
		if(id==BLOG_TITLE)
			c["blog_name"]=val;
		else if(id==BLOG_DESCRIPTION)
			c["blog_description"]=val;
		else if(id==BLOG_CONTACT)
			c["blog_contact"]=val;
	}
	blog->sql<<"SELECT value FROM text_options WHERE id='copyright'";
	if(blog->sql.single(r)) {
		string val;
		r>>val;
		c["copyright_string"]=val;
	}
	c["master_content"]=string("admin_editoptions");
	c["post_options_url"]=blog->fmt.edit_options;
}

void View_Admin::ini_login()
{
	ini_share();
	c["master_content"]=string("admin_login");
	c["login_url"]=blog->fmt.login;
}

void View_Admin::ini_cedit(int id)
{
	ini_share();

	c["master_content"]=string("admin_editcomment");
	c["edit_comment_url"]=str(format(blog->fmt.update_comment) % id);
	c["id"]=id;
	string author,url,content,email;
	blog->sql<<
		"SELECT author,url,email,content "
		"FROM	comments "
		"WHERE	id=?",id;
	row r;
	if(!blog->sql.single(r)) {
		throw Error(Error::E404);
	}
	r>>author>>url>>email>>content;
	c["author"]=author;
	c["url"]=url;
	c["content"]=content;
	c["email"]=email;
}

void View_Admin::ini_edit(int id,string ptype)
{
	ini_share();
	c["master_content"]=string("admin_editpost");
	View_Admin_Post post(blog,c);
	post.ini(id,ptype);
}

void View_Admin::ini_main()
{
	ini_share();
	View_Admin_Main main(blog,c);
	main.ini();
	c["master_content"]=string("admin_main");
}

void View_Admin_Main::ini()
{
	result rs;

	blog->sql<<
		"SELECT id,title "
		"FROM posts "
		"WHERE is_open=0";
	blog->sql.fetch(rs);
	row r;

	content::vector_t &unpublished_posts=c.vector("posts",rs.rows());

	int i;
	for(i=0;rs.next(r);i++) {
		int id;
		string intitle;
		r>>id>>intitle;
		string edit_url=str(format(blog->fmt.edit_post) % id);

		unpublished_posts[i]["edit_url"]=edit_url;
		if(intitle!="")
			unpublished_posts[i]["title"]=intitle;
	}

	blog->sql<<
		"SELECT id,title,is_open "
		"FROM	pages ";
	blog->sql.fetch(rs);

	content::vector_t &pages=c.vector("pages",rs.rows());

	for(i=0;rs.next(r);i++) {
		int id,is_open;
		string title;
		r>>id>>title>>is_open;
		string edit_url=str(format(blog->fmt.edit_page) % id);

		pages[i]["edit_url"]=edit_url;
		if(title!="")
			pages[i]["title"]=title;
		pages[i]["published"]=bool(is_open);
	}
	blog->sql<<
		"SELECT id,post_id,author "
		"FROM comments "
		"ORDER BY id DESC "
		"LIMIT 10",rs;

	content::vector_t &latest_comments=c.vector("comments",rs.rows());

	for(i=0;rs.next(r);i++) {
		int id,c_id;
		string author;
		r>>c_id>>id>>author;

		latest_comments[i]["post_permlink"]=str(format(blog->fmt.post) % id);
		latest_comments[i]["username"]=author;
		latest_comments[i]["edit_url"]=str(format(blog->fmt.edit_comment) % c_id );
	}
}

void View_Admin_Post::ini(int id,string ptype)
{
	bool is_post=ptype=="post";
	post_t post_data;
	c["is_page"]=(bool)(!is_post);

	if(is_post){
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

			content::vector_t &cats_in=c.vector("cats_in",res.rows());
			int i;
			for(i=0;res.next(r);i++) {
				int cid;
				string name;
				r>>cid>>name;
				inset.insert(cid);
				cats_in[i]["id"]=cid;
				cats_in[i]["name"]=name;
			}
		}

		content::list_t &cats_out=c.list("cats_out");
		blog->sql<<"SELECT id,name FROM cats",res;
		while(res.next(r)) {
			int cid;
			string name;
			r>>cid>>name;
			if(inset.find(cid)==inset.end()) {
				cats_out.push_back(content());
				cats_out.back()["id"]=cid;
				cats_out.back()["name"]=name;
			}
		}

	}

	if(id!=-1) {
		c["post_id"]=id;
		c["submit_post_url"]=str(format( is_post ? blog->fmt.update_post : blog->fmt.update_page) % id);
		if(is_post) {
			c["preview_url"]=str(format(blog->fmt.preview) % id);
			c["send_trackback_url"]=blog->fmt.send_trackback;
		}
		else
			c["preview_url"]=str(format(blog->fmt.page_preview) % id);
	}
	else {
		if(is_post)
			c["submit_post_url"]=blog->fmt.add_post;
		else
			c["submit_post_url"]=blog->fmt.add_page;
		return;
	}
	post_data.id=-1;
	row r;
	int is_open;
	if(is_post){
		blog->sql<<
			"SELECT id,title,abstract,content,is_open "
			"FROM posts "
			"WHERE id=?",
			id;
		if(!blog->sql.single(r)){
			throw Error(Error::E404);
		}
		r>>	post_data.id>>post_data.title>>
			post_data.abstract>>post_data.content>>is_open;
	}
	else {
		blog->sql<<
			"SELECT id,title,content,is_open "
			"FROM pages "
			"WHERE id=?",
			id;
		if(!blog->sql.single(r)){
			throw Error(Error::E404);
		}
		r>>	post_data.id>>post_data.title>>post_data.content>>is_open;
	}

	c["post_title"]=post_data.title;
	if(is_post)
		c["abstract"]=post_data.abstract;
	c["content"]=post_data.content;
	c["is_open"]=bool(is_open);
}


void View_Admin::ini_links()
{
	ini_share();
	result res;
	row r;
	blog->sql<<
		"SELECT id,name FROM link_cats",res;
	content::vector_t &cats=c.vector("link_cats",res.rows());
	int i;
	for(i=0;res.next(r);i++) {
		int id;
		string name;
		r>>id>>name;
		cats[i]["id"]=id;
		cats[i]["del"]=true;
		cats[i]["name"]=name;
	}

	c["master_content"]=string("admin_editlinks");
	c["submit_url"]=blog->fmt.edit_links;

	blog->sql<<
		"SELECT id,cat_id,title,url,description "
		"FROM links "
		"ORDER BY cat_id",res;
	content::vector_t &links=c.vector("links",res.rows());

	for(i=0;res.next(r);i++) {
		int id,cat_id;
		string title,url,description;
		r>>id>>cat_id>>title>>url>>description;
		links[i]["id"]=id;
		links[i]["cat_id"]=cat_id;
		links[i]["name"]=title;
		links[i]["url"]=url;
		if(description!="")
			links[i]["descr"]=description;
	}
}


void View_Admin::ini_cats()
{
	ini_share();
	result res;
	row r;
	blog->sql<<"SELECT id,name FROM cats",res;
	content::vector_t &cats=c.vector("cats",res.rows());
	int i;
	for(i=0;res.next(r);i++) {
		int id;
		string name;
		r>>id>>name;
		cats[i]["id"]=id;
		cats[i]["name"]=name;
	}

	c["master_content"]=string("admin_editcats");
	c["submit_url"]=blog->fmt.edit_cats;
}
