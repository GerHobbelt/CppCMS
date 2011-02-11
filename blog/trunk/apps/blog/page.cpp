#include <apps/blog/page.h>
#include <data/blog/page.h>

#include <cppcms/url_mapper.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/cache_interface.h>

#include <cppdb/frontend.h>

namespace apps {
namespace blog {
	
page::page(cppcms::service &s) : master(s)
{
	dispatcher().assign("/(\\d+)",&page::display,this,1);
	mapper().assign("/{1}");
	dispatcher().assign("/(\\d+)/preview",&page::preview,this,1);
	mapper().assign("preview","/{1}/preview");
}

void page::preview(std::string id)
{
	if(user_.empty()) {
		response().make_error_response(cppcms::http::response::forbidden);
		return;
	}
	prepare(id,true);
}

void page::display(std::string id)
{
	std::string key = "page_" + id;
	if(cache().fetch_page(key))
		return;
	if(!prepare(id,false))
		return;
	cache().store_page(key);
}

bool page::prepare(std::string const &sid,bool preview)
{
	int id = atoi(sid.c_str());
	
	cppdb::result r = sql() <<
		"SELECT title,content,is_open "
		"FROM	pages "
		"WHERE	id=?" << id << cppdb::row;

	if(r.empty()) {
		response().make_error_response(404);
		return false;
	}
	data::blog::page c;
	c.id = id;
	int is_open = 0;
	r >> c.title >> c.content >> is_open;
	if(!preview && !is_open) {
		response().make_error_response(404);
		return false;
	}
	master::prepare(c);
	render("page",c);
	return true;
}

} // blog
} // apps
