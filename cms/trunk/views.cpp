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

void View_Comment::init(comment_t &c)
{
	Text_Tool tt;
	user_t user;
	if(c.author_id!=-1 && users->id.get(c.author_id,user)) {
		tt.text2html(user.username,author);
	}
	else if(!(c.author=="")) {
		tt.text2html(c.author,author);
		if(c.url[0]!=0){
			tt.text2url(c.url,url);
		}
	}
	else {
		author="unknown";
	}
	string text;
	texts->get(c.content_id,text);
	tt.markdown2html(text,message);
	blog->date(c.publish_time,date);
}

int View_Comment::render(Renderer &r,Content &c, string &out)
{
	c[TV_username]=author;
	c[TV_content]=message;
	c[TV_date]=date;
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
	blog->date(p.publish,date);
	user_t user;
	if(users->id.get(p.author_id,user)) {
		tt.text2html(user.username,author);
	}
	permlink=str(format(blog->fmt.post) % p.id);
}

void View_Post::ini_full(post_t &p)
{
	Text_Tool tt;
	ini_share(p);
	string abstract;
	string content;
	texts->get(p.abstract_id,abstract);
	tt.markdown2html(abstract,this->abstract);
	if(p.content_id!=-1) {
		texts->get(p.content_id,content);
		tt.markdown2html(content,this->content);
		has_content=true;
	}
	else {
		has_content=false;
	}

	string post_comment=str(format(blog->fmt.add_comment) % p.id);
	int n=post_comment.size()/2;
	post_comment2=post_comment.substr(n);
	post_comment1=post_comment.substr(0,n);

	Comments::posttime_c cur(comments->posttime);
	has_comments=false;
	for(cur.gte(comment_t::sec_t(p.id,0));
		    cur && cur.val().post_id==p.id;
		    cur.next())
	{
		if(!cur.val().moderated) {
			continue;
		}
		shared_ptr<View_Comment> com(new View_Comment(blog));
		comments_list.push_back(com);
		comment_t comment=cur;
		com->init(comment);
		has_comments=true;
	}
}


void View_Post::ini_short(post_t &p)
{
	Text_Tool tt;
	ini_share(p);
	string abstract;
	texts->get(p.abstract_id,abstract);
	tt.markdown2html(abstract,this->abstract);
	if(p.content_id!=-1) {
		has_content=true;
	}
	else {
		has_content=false;
	}
	has_comments=false;
}


void View_Post::ini_feed(post_t &p)
{
	Text_Tool tt;
	ini_share(p);
	string abstract,abstract_html;
	texts->get(p.abstract_id,abstract);
	tt.markdown2html(abstract,abstract_html);
	// For xml feed we need convert html to text
	tt.text2html(abstract_html,this->abstract);
	has_content=has_comments=false;
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
	option_t opt;

	options->get(BLOG_TITLE,opt);
	title=(char const*)opt.value;
	options->get(BLOG_DESCRIPTION,opt);
	description=(char const*)opt.value;

}

void View_Main_Page::ini_main(int id)
{
	ini_share();

	Posts::publish_c cur(posts->publish);

	if(id==-1) {
		cur.end();
	}
	else {
		post_t p;
		posts->id.get(id,p);
		cur.lte(p.publish);
	}

	int counter=0;

	int max_posts=5;

	latest_posts.reserve(max_posts);

	for(;;) {
		if(cur) {
			if(!cur.val().is_open){
				cur.next();
				continue;
			}
			if(counter==max_posts) {
				from=str(format(blog->fmt.main_from) % cur.val().id);
				break;
			}
			post_t post;
			cur.get(post);

			shared_ptr<View_Post> ptr(new View_Post(blog));
			ptr->ini_short(post);
			latest_posts.push_back(ptr);
			counter++;
			cur.next();
		}
		else {
			break;
		}
	}
	disp=SUMMARY;
}

void View_Main_Page::ini_post(int id,bool preview)
{
	shared_ptr<View_Post> ptr(new View_Post(blog));
	post_t post;
	if(!(posts->id.get(id,post)) || (!post.is_open && !preview)){
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
	option_t opt;
	options->get(BLOG_TITLE,opt);
	blog_name=(char const*)opt.value;
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
	Posts::is_open_c cur(posts->is_open);
	for(cur<=false;cur;cur.next()) {
		post_ref post_data;
		post_t const &post=cur;
		post_data.edit_url=
			str(format(blog->fmt.edit_post) % post.id);
		tt.text2html(post.title,post_data.title);
		unpublished_posts.push_back(post_data);
	}
}

int View_Admin_Main::render(Renderer &r,Content &c,string &out)
{
	c[TV_new_post_url]=blog->fmt.new_post;
	int id=r.render(out);
	list<post_ref>::iterator it=unpublished_posts.begin();
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
		else
			return id;
		id=r.render(out);
	}
}

void View_Admin_Post::ini( int id)
{
	Text_Tool tt;
	post_t post_data;
	if(id!=-1) {
		post_id=str(format("%1%") % id);
		post_url=str(format(blog->fmt.update_post) % id);
		preview=str(format(blog->fmt.preview) % id);
	}
	else {
		post_url=blog->fmt.add_post;
		return;
	}
	if(!posts->id.get(id,post_data)){
		throw Error(Error::E404);
	}
	title=post_data.title.c_str();
	string abs_tmp,cnt_tmp;
	if(post_data.abstract_id!=-1 && texts->get(post_data.abstract_id,abs_tmp)) {
		tt.text2html(abs_tmp,abstract);
	}
	if(post_data.content_id!=-1 && texts->get(post_data.content_id,cnt_tmp)) {
		tt.text2html(cnt_tmp,content);
	}
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

