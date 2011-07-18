#include <apps/feed/feed.h>
#include <data/feed.h>

#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>
#include <cppdb/frontend.h>

namespace apps {
namespace feed {
	feed_master::feed_master(cppcms::service &s) : basic_master(s)
	{
		mapper().assign("posts","");
		dispatcher().assign("/?",&feed_master::posts,this);

		mapper().assign("posts","/cat/{1}/");
		dispatcher().assign("/cat/(\\d+)/?",&feed_master::cats,this,1);

		mapper().assign("comments","/comments");
		dispatcher().assign("/comments/?",&feed_master::comments,this);
	}
	
	void feed_master::comments()
	{
		// make correct header for cached feed too
		response().set_content_header("text/xml");

		if(cache().fetch_page("comments"))
			return;
		cppdb::result r;
		r = sql()<<
			"SELECT id,post_id,author,content "
			"FROM comments "
			"ORDER BY id DESC "
			"LIMIT 10";
		data::feed::comments c;
		basic_master::prepare(c);
		c.items.reserve(10);
		int i;
		for(i=0;r.next();i++){
			c.items.resize(i+1);
			data::feed::comment &cm = c.items.back();
			r>>cm.id>>cm.post_id>>cm.author>>cm.content;
		}
		render("feed","comments",c);
		cache().store_page("comments");
	}
	void feed_master::posts()
	{
		if(cache().fetch_page("rss_posts"))
			return;
		cache().add_trigger("cat_0");

		cppdb::result r;
		r = sql() <<
			"SELECT id,title,abstract,length(posts.content) "
			"FROM	posts "
			"WHERE	posts.is_open=1 "
			"ORDER BY posts.publish DESC "
			"LIMIT 10";
		data::feed::posts c;
		c.items.reserve(10);
		while(r.next()) {
			c.items.push_back(data::feed::post());
			data::feed::post &last = c.items.back();
			int size;
			r >> last.id >> last.title >> last.abstract >> size;
			last.has_content = size != 0;
			std::ostringstream ss;
			ss << "post_" << last.id;
			cache().add_trigger(ss.str());
		}
		basic_master::prepare(c);
		response().set_content_header("text/xml");
		render("feed","posts",c);
		cache().store_page("rss_posts");

	}
	void feed_master::cats(std::string sid)
	{
		std::string key = "rss_" + sid;
		if(cache().fetch_page(key))
			return;
		cache().add_trigger("cat_" + sid);

		int id=atoi(sid.c_str());
		cppdb::result r;
		r = sql() << "SELECT name FROM cats WHERE id = ?" << id << cppdb::row;
		if(r.empty()) {
			response().make_error_response(404);
			return;
		}
		
		data::feed::posts c;
		r >> c.category;
		c.id = id;
		
		r = sql() <<
			"SELECT posts.id,posts.title, "
			"	posts.abstract, length(posts.content) "
			"FROM	post2cat "
			"LEFT JOIN	posts ON post2cat.post_id=posts.id "
			"WHERE	post2cat.cat_id=? "
			"	AND post2cat.is_open=1 "
			"ORDER BY post2cat.publish DESC "
			"LIMIT 10" 
			<< id;
		c.items.reserve(10);
		while(r.next()) {
			c.items.push_back(data::feed::post());
			data::feed::post &last = c.items.back();
			int size;
			r >> last.id >> last.title >> last.abstract >> size;
			last.has_content = size != 0;
			std::ostringstream ss;
			ss << "post_" << last.id;
			cache().add_trigger(ss.str());
		}
		basic_master::prepare(c);
		response().set_content_header("text/xml");
		render("feed","posts",c);
		cache().store_page(key);
	}
}
}
