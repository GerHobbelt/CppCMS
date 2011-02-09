#include <apps/blog/post.h>
#include <data/blog/post.h>

#include <cppcms/url_mapper.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/cache_interface.h>

#include <booster/posix_time.h>

#include <cppdb/frontend.h>

namespace apps {
namespace blog {

post::post(cppcms::service &srv) : master(srv)
{
	dispatcher().assign("/(\\d+)",&post::display,this,1);
	mapper().assign("/{1}");
	dispatcher().assign("/(\\d+)/preview",&post::preview,this,1);
	mapper().assign("preview","/{1}/preview");
	dispatcher().assign("/(\\d+)/postback",&post::postback,this,1);
	mapper().assign("postback","/{1}/postback");
}

void post::postback(std::string sid)
{
	int id = atoi(sid.c_str());
	if(request().request_method()!="POST") {
		response().make_error_response(404);
		return;
	}
	data::blog::post c;
	c.response.load(context());
	if(c.response.validate()) {
		cppdb::transaction tr(sql());
		cppdb::result r=sql()<<"SELECT is_open FROM posts WHERE id=?" << id << cppdb::row;
		if(r.empty()) {
			response().make_error_response(404);
			return;
		}
		int is_open;
		r >> is_open;
		r.clear();
		if(!is_open) {
			response().make_error_response(404);
			return;
		}
		
		if(c.response.send.value()) {
			std::tm publish_time = booster::ptime::local_time(booster::ptime::now());

			sql()<<	"INSERT INTO comments(post_id,author,email,url,publish_time,content) "
				"VALUES(?,?,?,?,?,?)" 
				<< id 
				<< c.response.author.value()
				<< c.response.mail.value()
				<< c.response.url.value()
				<< publish_time 
				<< c.response.content.value() 
				<< cppdb::exec;

			sql()<< "UPDATE posts "
				"SET comment_count = (SELECT count(*) FROM comments WHERE post_id=?)"
				"WHERE id = ?" << id << cppdb::exec;

			std::ostringstream ss;
			ss << "post_" << id;
			cache().rise("comments");
			cache().rise(ss.str());
			tr.commit();
			response().set_redirect_header(url("/blog/post",id));
		}
		else {
			c.preview_content = c.response.content.value();
		}
	}
	if(prepare_content(c,sid,false))
		render("post",c);
}

void post::preview(std::string id)
{
	if(user_.empty()) {
		response().make_error_response(403);
		return;
	}
	data::blog::post c;
	if(!prepare_content(c,id,true))
		return;
	render("post",c);
}

void post::display(std::string id)
{
	std::string key = "post_" + id;
	if(cache().fetch_page(key))
		return;
	data::blog::post c;
	if(!prepare_content(c,id,false))
		return;
	render("post",c);
	cache().store_page(key);
}

bool post::prepare_content(data::blog::post &c,std::string const &sid,bool preview)
{
	int id = atoi(sid.c_str());
	cppdb::result r;
	r = sql() << 
		"SELECT title,abstract,content,publish,is_open,users.username "
		"FROM posts "
		"JOIN users on posts.author_id = users.id "
		"WHERE posts.id = ?" << id << cppdb::row; 
	if(r.empty()) {
		response().make_error_response(404);
		return false;
	}
	int is_open = 0;
	std::tm published_tm=std::tm();
	r >> c.title >> c.abstract >> c.content >>published_tm >> is_open >> c.author;
	c.published = mktime(&published_tm);
	if(!is_open && !preview) {
		response().make_error_response(403);
		return false;
	}
	c.id = id;
	master::prepare(c);
	r = sql()<<
		"SELECT id,author,url,publish_time,content "
		"FROM comments "
		"WHERE post_id=? "
		"ORDER BY publish_time" << id;
	
	c.comments.reserve(10);

	int i;
	for(i=0;r.next();i++)
	{
		std::tm date;
		c.comments.resize(i+1);
		data::blog::comment &com=c.comments.back();
		r	>> com.id >>com.username >> com.url
			>> date >> com.content ;
		com.date=mktime(&date);
	}
	c.comment_count = i;
	r = sql()<<
		"SELECT post2cat.cat_id,cats.name "
		"FROM	post2cat "
		"JOIN	cats ON post2cat.cat_id=cats.id "
		"WHERE	post2cat.post_id=?" << id;
	for(i=0;r.next();i++) {
		data::blog::category cat;
		r>>cat.id>>cat.name;
		c.categories.push_back(cat);
	}
	return true;
}


} // blog
} // apps
