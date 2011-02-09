#include <apps/blog/summary.h>
#include <data/blog/summary.h>

#include <cppdb/frontend.h>
#include <cppcms/http_response.h>
#include <cppcms/url_mapper.h>
#include <cppcms/url_dispatcher.h>

namespace apps {
namespace blog {

summary::summary(cppcms::service &s) : master(s)
{
	mapper().assign("");
	dispatcher().assign("/?",&summary::all,this);

	mapper().assign("/from/{1}");
	dispatcher().assign("/from/(\\d+)/?",&summary::all,this,1);

	mapper().assign("category","/cat/{1}");
	dispatcher().assign("/cat/(\\d+)/?",&summary::category,this,1);

	mapper().assign("category","/cat/{1}/from/{2}");
	dispatcher().assign("/cat/(\\d+)/from/(\\d+)/?",&summary::category,this,1,2);
}

void summary::all()
{
	prepare(0,0);
}

void summary::all(std::string page)
{
	prepare(0,atoi(page.c_str()));
}

void summary::category(std::string id)
{
	prepare(atoi(id.c_str()),0);
}

void summary::category(std::string id,std::string page)
{
	prepare(atoi(id.c_str()),atoi(page.c_str()));
}


bool summary::prepare(int cat_id,int page)
{
	static const int page_size = 5;
	cppdb::result r;
	
	data::blog::summary c;
	
	if(cat_id != 0) {
		r = sql() << "SELECT name FROM cats WHERE id=?" << cat_id << cppdb::row;
		if(r.empty()) {
			response().make_error_response(404);
			return false;
		}
		r >> c.category_name;
		c.id = cat_id;
		r = sql() <<
			"SELECT posts.id,users.username,posts.title, "
			"	posts.abstract, length(posts.content), "
			"	posts.publish,posts.comment_count "
			"FROM	post2cat "
			"LEFT JOIN	posts ON post2cat.post_id=posts.id "
			"LEFT JOIN	users ON users.id=posts.author_id "
			"WHERE	post2cat.cat_id=? "
			"	AND post2cat.is_open=1 "
			"ORDER BY post2cat.publish DESC "
			"LIMIT ? OFFSET ?" 
			<< cat_id << (page_size + 1) << (page * page_size);
	}
	else {
		r = sql() <<
			"SELECT posts.id,users.username,posts.title, "
			"	posts.abstract, length(posts.content), "
			"	posts.publish,posts.comment_count "
			"FROM	posts "
			"LEFT JOIN "
			"	users ON users.id=posts.author_id "
			"WHERE	posts.is_open=1 "
			"ORDER BY posts.publish DESC "
			"LIMIT ? OFFSET ? " 
			<< (page_size + 1) << (page * page_size);
	}


	c.next_page = page + 1;
	c.page = page;
	c.prev_page = page > 0 ? page - 1 : 0;

	c.posts.reserve(page_size);

	int current_pos = 0;
	while(r.next()) {
		if(current_pos >= page_size) {
			c.next_page = page + 1;
			break;
		}
		c.posts.resize(current_pos + 1);
		data::blog::post_content &cur = c.posts.back();
		int content_size = 0;
		std::tm published=std::tm();
		r 	>> cur.id >> cur.author >> cur.title >> cur.abstract >> content_size 
			>> published >> cur.comment_count;
		cur.has_content = content_size != 0;
		cur.published = mktime(&published);
		current_pos ++;
	}

	master::prepare(c);

	if(!c.posts.empty()) {
		std::string in_close;
		in_close.reserve(c.posts.size() * 2);
		for(unsigned i=0;i<c.posts.size();i++)  {
			if(i == 0)
				in_close+="?";
			else
				in_close+=",?";
		}
		cppdb::statement st = sql() << 
			"SELECT   post_id, cat_id,cats.name "
			"FROM     post2cat "
			"JOIN     cats ON cats.id = post2cat.cat_id "
			"WHERE    post_id in (" + in_close + ") "
			"ORDER BY post_id";
		std::map<int,int> id2pos;
		for(unsigned i=0;i<c.posts.size();i++) {
			int post_id = c.posts[i].id;
			st.bind(post_id);
			id2pos[post_id] = i;
		}
		r = st.query();
		while(r.next()) {
			int post_id;
			data::blog::category cat;
			r  >> post_id >> cat.id >> cat.name;
			c.posts.at(id2pos[post_id]).categories.push_back(cat);
		}
	}

	render("summary",c);

	return true;
}



} // 
} // 
