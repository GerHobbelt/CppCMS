#include <apps/admin/summary.h>
#include <data/admin/summary.h>
#include <cppcms/url_mapper.h>
#include <cppcms/url_dispatcher.h>
#include <cppdb/frontend.h>

namespace apps {
namespace admin {

summary::summary(cppcms::service &s) : master(s)
{
	mapper().assign("");
	dispatcher().assign("/?",&summary::prepare,this);
}

void summary::prepare()
{
	data::admin::summary c;
	master::prepare(c);
	cppdb::result r;
	r= sql() << 
		"SELECT id,title "
		"FROM posts "
		"WHERE is_open = 0";
	c.posts.reserve(10);
	while(r.next()) {
		c.posts.push_back(data::admin::summary::post());
		data::admin::summary::post &last = c.posts.back();
		r >> last.id >> last.title;
	}
	c.pages.reserve(10);
	r = sql() <<"SELECT id,title,is_open FROM pages";
	while(r.next()) {
		c.pages.push_back(data::admin::summary::page());
		data::admin::summary::page &last = c.pages.back();
		r >> last.id >> last.title >> last.is_open;
	}
	r = sql() <<                
		"SELECT id,post_id,author "
		"FROM comments "
		"ORDER BY id DESC "
		"LIMIT 10";
	c.comments.reserve(10);
	while(r.next()) {
		c.comments.push_back(data::admin::summary::comment());
		data::admin::summary::comment &last = c.comments.back();
		r >> last.id >> last.post_id >> last.username;
	}

	render("admin_skin","summary",c);

}


} // admin
} // apps
