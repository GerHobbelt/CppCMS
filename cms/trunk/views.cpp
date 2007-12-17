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


void View_Comment::init(comment_t &c)
{
	Text_Tool tt;
	user_t user;
	if(c.author_id!=-1 && users->get(id,user)) {
		tt.text2html(user.username,author);
	}
	else if(!(c.author=="")) {
		tt.text2html(c.author,author);
		url=c.url;
	}
	else {
		author="unknown";
	}
	string text;
	texts->get(c.content_id,text);
	tt.markdown2html(text,message);
	blog->date(c.published,date);
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
	return r.next(out);
}

void View_Post::ini_share(post_t &p)
{
	Text_Tool tt;
	tt.text2html(p.title,title);
	blog->date(p.published,date);
	user_t user;
	if(users->get(p.author_id,user)) {
		tt.text2html(user.username,author);
	}
	permlink=str(format(blog->fmt.post) % p.id);
}

void View_Post::ini_full(post_t &p)
{
	ini_share(p);
	string abstract;
	string content;
	texts->get(p.abstract_id,abstract);
	tt.markdown2html(abstract,this->abstract);
	if(p.content_id!=-1) {
		texts->get(p.content_id,content);
		tt.markdown2html(content,this->content);
	}
	texts->get(p.abstract_id,abstract);
	post_comment=str(format(blog->fmt.add_post) % p.id);
	
	Comments::posttime_c cur(*comments);
	for(cur.gte(comment_t::sec_t(p.id,0));
		    cur && cur.val().post_id==p.id;
		    cur.next())
	{
		View_Comment com(blog);
		comments->push_back(com);
		list<View_Comment>::iterator ptr;
		ptr=comments.end();
		ptr->init(cur);
	}
}
