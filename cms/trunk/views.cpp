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
#include "templates/look.h"
#include <cppcms/text_tool.h>
#include <boost/format.hpp>
#include "blog.h"
#include "error.h"

using boost::format;
using boost::str;

using namespace soci;

void View_Comment::init(comment_t &c)
{
	Text_Tool tt;
	user_t user;
	tt.text2html(c.author,author);
	if(c.url.size()!=0){
		tt.text2url(c.url.c_str(),url);
	}
	tt.markdown2html(c.content,message);
	blog->date(c.publish_time,date);
	del_url=str(format(blog->fmt.del_comment) % c.id);
}

int View_Comment::render(Renderer &r,Content &c, string &out)
{
	c[TV_username]=author;
	c[TV_content]=message;
	c[TV_date]=date;
	if(blog->userid!=-1){
		c[TV_delete_url]=del_url;
	}
	if(url!="") {
		c[TV_url]=url;
	}
	else {
		c[TV_url].reset();
	}
	return r.render(out);
}

void View_Post::ini_share(post_t &p)
{
	Text_Tool tt;
	tt.text2html(p.title,title);
	blog->date( p.publish ,date);
	tt.text2html(p.author_name,author);

	edit_url=str(format(blog->fmt.edit_post) % p.id);
	permlink=str(format(blog->fmt.post) % p.id);
}

void View_Post::ini_full(post_t &p)
{
	Text_Tool tt;
	ini_share(p);
	tt.markdown2html(p.abstract,this->abstract);
	if(p.content!="") {
		tt.markdown2html(p.content,this->content);
		has_content=true;
	}
	else {
		has_content=false;
	}

	string post_comment=str(format(blog->fmt.add_comment) % p.id);
	int n=post_comment.size()/2;
	post_comment2=post_comment.substr(n);
	post_comment1=post_comment.substr(0,n);


	has_comments=false;

	rowset<> rs=(blog->sql.prepare<<
		"SELECT id,author,email,url,publish_time,content "
		"FROM comments "
		"WHERE post_id=:id "
		"ORDER BY publish_time",use(p.id));

	rowset<>::const_iterator cur;

	for(cur=rs.begin();cur!=rs.end();cur++)
	{
		shared_ptr<View_Comment> com(new View_Comment(blog));
		comments_list.push_back(com);
		comment_t comment;
		*cur	>> comment.id >>comment.author >> comment.email >> comment.url
			>> comment.publish_time >> comment.content ;
		com->init(comment);
		has_comments=true;
	}
}


void View_Post::ini_short(post_t &p)
{
	Text_Tool tt;
	ini_share(p);
	tt.markdown2html(p.abstract,abstract);
	has_content=p.has_content;
	has_comments=false;
}


void View_Post::ini_feed(post_t &p)
{
	Text_Tool tt;
	ini_share(p);
	string abstract_html;
	tt.markdown2html(p.abstract,abstract_html);
	// For xml feed we need convert html to text
	tt.text2html(abstract_html,this->abstract);
	has_content=p.has_content;
	has_comments=false;
}

int View_Post::render(Renderer &r,Content &c, string &out)
{
	c[TV_post_permlink]=permlink;
	c[TV_post_title]=title;
	c[TV_date]=date;
	c[TV_author]=author;
	c[TV_post_abstract]=abstract;
	c[TV_post_comment_url_1]=post_comment1;
	c[TV_post_comment_url_2]=post_comment2;
	c[TV_has_content]=has_content;

	if(blog->userid!=-1) {
		c[TV_edit_url]=edit_url;
	}

	if(has_content) {
		c[TV_content]=content;
	}
	else {
		c[TV_content].reset();
	}

	list<shared_ptr<View_Comment> >::iterator i=comments_list.begin();

	id=r.render(out);
	while(id) {
		if(id==TV_get_comment) {
			if(has_comments && i!=comments_list.end()) {
				c[TV_next_comment]=true;
				id=(*i)->render(r,c,out);
				i++;
			}
			else {
				c[TV_next_comment]=false;
				return r.render(out);
			}
		}
		else {
			return id;
		}
	}
}

void View_Main_Page::ini_share()
{
	int id;
	string val;
	rowset<> rs=(blog->sql.prepare<<"SELECT id,value FROM options");
	rowset<>::const_iterator i;
	for(i=rs.begin();i!=rs.end();i++){
		*i >>id>>val;
		if(id==BLOG_TITLE)
			title=val;
		else if(id==BLOG_DESCRIPTION)
			description=val;
	}
}

void View_Main_Page::ini_main(int id,bool feed)
{
	ini_share();

	int max_posts=feed ? 10 : 5;

	latest_posts.reserve(max_posts);

	rowset<> rs =
		(id!=-1) ?
		(blog->sql.prepare<<
			"SELECT posts.id,users.username,posts.title, "
			"	posts.abstract, posts.content !='', "
			"	posts.publish "
			"FROM	posts "
			"JOIN	users ON users.id=posts.author_id "
			"WHERE	posts.publish >= (SELECT publish FROM posts WHERE id=:id) "
			"	AND posts.is_open=1 "
			"ORDER BY posts.publish DESC "
			"LIMIT :max",use(max_posts+1),use(id))
		:
		(blog->sql.prepare<<
			"SELECT posts.id,users.username,posts.title, "
			"	posts.abstract, posts.content!='', "
			"	posts.publish "
			"FROM	posts "
			"JOIN	users ON users.id=posts.author_id "
			"WHERE	posts.is_open=1 "
			"ORDER BY posts.publish DESC "
			"LIMIT :max",use(max_posts+1));

	rowset<>::const_iterator row;
	int counter;
	for(counter=0,row=rs.begin();row!=rs.end();row++,counter++) {
		if(counter==max_posts+1) {
			int id;
			*row>>id;
			from=str(format(blog->fmt.main_from) % id);
			break;
		}
		post_t post;
		*row>>post.id;
		*row>>post.author_name;
		*row>>post.title;
		*row>>post.abstract;
		// The type of output varies
		// for different DBS WTF!!!
		if(row->get_properties(4).get_data_type()==eInteger){
			*row>>post.has_content;
		}
		else if(row->get_properties(4).get_data_type()==eLongLong){
			long long tmp;
			*row>>tmp;
			post.has_content=(int)tmp;
		}
		else {
			string tmp;
			*row>>tmp;
			post.has_content=atoi(tmp.c_str());
		}

		*row>>post.publish;

		shared_ptr<View_Post> ptr(new View_Post(blog));
		if(feed){
			ptr->ini_feed(post);
		}
		else {
			ptr->ini_short(post);
		}
		latest_posts.push_back(ptr);
	}
	disp=SUMMARY;
}

void View_Main_Page::ini_post(int id,bool preview)
{
	shared_ptr<View_Post> ptr(new View_Post(blog));
	post_t post;
	eIndicator ind;
	post.id=-1;
	blog->sql<<
		"SELECT posts.id,users.username,posts.title, "
		"	posts.abstract, posts.content, "
		"	posts.publish,posts.is_open "
		"FROM	posts "
		"JOIN	users ON users.id=posts.author_id "
		"WHERE	posts.id=:id ",
		use(id),
		into(post.id),
		into(post.author_name),into(post.title),
		into(post.abstract),into(post.content,ind),
		into(post.publish),into(post.is_open);

	if(post.id==-1 || (!post.is_open && !preview)){
		throw Error(Error::E404);
	}

	single_post=ptr;

	ini_share();

	ptr->ini_full(post);
	disp=SINGLE;
}

void View_Main_Page::ini_error(int id)
{
	ini_share();
	disp=ERROR;
	error_code=id;
}


int View_Main_Page::render( Renderer &r,Content &c,string &out)
{
	c[TV_media]=blog->fmt.media;
	c[TV_title]=title;
	c[TV_blog_name]=title;
	c[TV_blog_description]=description;
	c[TV_admin_url]=blog->fmt.admin;
	c[TV_base_url]=global_config.sval("blog.script_path").c_str();
	c[TV_host]=global_config.sval("blog.host").c_str();
	c[TV_rss_posts]=blog->fmt.feed;
	if(from!="") {
		c[TV_next_page_link]=from;
	}
	if(disp==SINGLE) {
		c[TV_subtitle]=single_post->title;
		c[TV_master_content]=TT_post;
		return single_post->render(r,c,out);
	}
	else if(disp==SUMMARY){
		c[TV_master_content]=TT_main_page;
		int id;
		id=r.render(out);
		vector<shared_ptr<View_Post> >::iterator i=latest_posts.begin();
		for(;;){
			if(id==TV_get_post){
				if(i!=latest_posts.end()) {
					c[TV_next_post]=true;
					id=(*i)->render(r,c,out);
					i++;
				}
				else {
					c[TV_next_post]=false;
					id=r.render(out);
				}
			}
			else {
				return id;
			}
		}
	}
	else if(disp==ERROR){
		c[TV_master_content]=TT_error;
		switch(error_code) {
		case Error::E404:
			c[TV_error_404]="";
			break;
		case Error::COMMENT_FIELDS:
			c[TV_error_comment]="";
			break;
		}
		return r.render(out);
	}
}


void View_Admin::ini_share()
{
	blog->sql<<
		"SELECT value FROM options WHERE id=:id",
		use((int)BLOG_TITLE),into(blog_name);
}

void View_Admin::ini_login()
{
	ini_share();
	page=LOGIN;
}

void View_Admin::ini_edit(int id)
{
	ini_share();
	page=POST;
	post=shared_ptr<View_Admin_Post>(new View_Admin_Post(blog));
	post->ini(id);
}

void View_Admin::ini_main()
{
	ini_share();
	main=shared_ptr<View_Admin_Main>(new View_Admin_Main(blog));
	main->ini();
	page=MAIN;
}

int View_Admin::render( Renderer &r,Content &c,string &out)
{
	c[TV_media]=blog->fmt.media;
	c[TV_base_url]=global_config.sval("blog.script_path").c_str();
	c[TV_admin_url]=blog->fmt.admin.c_str();
	c[TV_logout_url]=blog->fmt.logout;
	c[TV_blog_name]=blog_name;
	switch(page){
	case MAIN:
		c[TV_master_content]=TT_admin_main;
		return main->render(r,c,out);
	case POST:
		c[TV_master_content]=TT_admin_editpost;
		return post->render(r,c,out);
	case LOGIN:
		c[TV_master_content]=TT_admin_login;
		c[TV_login_url]=blog->fmt.login.c_str();
		return r.render(out);
	}
	return r.render(out);
}

void View_Admin_Main::ini()
{
	Text_Tool tt;
	rowset<>::const_iterator r;
	rowset<> rs1=(blog->sql.prepare<<
		"SELECT id,title "
		"FROM posts "
		"WHERE is_open=0");
	for(r=rs1.begin();r!=rs1.end();r++) {
		int id;
		string title;
		*r>>id>>title;
		post_ref post_data;
		post_data.edit_url=
			str(format(blog->fmt.edit_post) % id);
		tt.text2html(title,post_data.title);
		unpublished_posts.push_back(post_data);
	}
	rowset<> rs2=(blog->sql.prepare<<
		"SELECT post_id,author "
		"FROM comments "
		"ORDER BY publish_time DESC "
		"LIMIT 10");

	for(r=rs2.begin();r!=rs2.end();r++) {
		int id;
		string author;
		*r>>id>>author;
		comment_ref com;
		com.url=str(format(blog->fmt.post) % id);
		tt.text2html(author,com.author);
		latest_comments.push_back(com);
	}
}

int View_Admin_Main::render(Renderer &r,Content &c,string &out)
{
	c[TV_new_post_url]=blog->fmt.new_post;
	int id=r.render(out);
	list<post_ref>::iterator it=unpublished_posts.begin();
	list<comment_ref>::iterator it_c=latest_comments.begin();
	for(;;) {
		if(id==TV_get_post && it!=unpublished_posts.end()) {
			c[TV_next_post]=true;
			c[TV_post_title]=it->title.c_str();
			c[TV_edit_url]=it->edit_url;
			it++;
		}
		else if(id==TV_get_post){
			c[TV_next_post]=false;
		}
		else if(id==TV_get_comment && it_c!=latest_comments.end()){
			c[TV_next_comment]=true;
			c[TV_username]=it_c->author;
			c[TV_post_permlink]=it_c->url;
			it_c++;
		}
		else if(id==TV_get_comment) {
			c[TV_next_comment]=false;
		}
		else
			return id;
		id=r.render(out);
	}
}

void View_Admin_Post::ini( int id)
{
	Text_Tool tt;
	post_t post_data;
	eIndicator ind;
	if(id!=-1) {
		post_id=str(format("%1%") % id);
		post_url=str(format(blog->fmt.update_post) % id);
		preview=str(format(blog->fmt.preview) % id);
	}
	else {
		post_url=blog->fmt.add_post;
		return;
	}
	post_data.id=-1;
	blog->sql<<
		"SELECT id,title,abstract,content "
		"FROM posts "
		"WHERE id=:id",
		use(id),
		into(post_data.id),into(post_data.title),
		into(post_data.abstract),into(post_data.content,ind);
	if(post_data.id==-1){
		throw Error(Error::E404);
	}
	title=post_data.title;
	tt.text2html(post_data.abstract,abstract);
	tt.text2html(post_data.content,content);
}

int View_Admin_Post::render( Renderer &r,Content &c,string &out)
{
	c[TV_submit_post_url]=post_url.c_str();
	if(post_id.size()>0) {
		c[TV_post_id]=post_id.c_str();
		c[TV_preview_url]=preview.c_str();
	}
	c[TV_post_title]=title.c_str();
	c[TV_abstract]=abstract.c_str();
	c[TV_content]=content.c_str();
	return r.render(out);
}

