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

	latest_posts.reserve(10);

	for(;;) {
		if(cur) {
			if(!cur.val().is_open)
				continue;
			if(counter==10) {
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
		break;
	}
	disp=SUMMARY;
}

void View_Main_Page::ini_post(int id)
{
	shared_ptr<View_Post> ptr(new View_Post(blog));
	post_t post;
	if(!(posts->id.get(id,post))){
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
	c[TV_base_url]=global_config.sval("blog.script_path").c_str();
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
