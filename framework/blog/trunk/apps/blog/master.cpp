#include <apps/blog/master.h>
#include <data/blog/master.h>

#include <cppcms/json.h>
#include <cppcms/session_interface.h>
#include <cppcms/cache_interface.h>
#include <cppdb/frontend.h>


namespace apps {
namespace blog {



master::master(cppcms::service &s) : basic_master(s)
{
}

master::~master()
{
}

namespace {
	struct sidebar_init {
		master *m;
		data::blog::master  *data;
		void operator()() const
		{
			m->load_sidebar(*data);
		}
	};
}

void master::load_sidebar(data::blog::master &c)
{
	if(cache().fetch_data("sidebar_info",c.sidebar))
		return;
	
	cppdb::result r;

	r=sql()<<
		"SELECT	id,title "
		"FROM	pages "
		"WHERE	is_open=1";

	c.sidebar.pages.reserve(16);
	while(r.next()) {
		c.sidebar.pages.resize(c.sidebar.pages.size()+1);
		r >> c.sidebar.pages.back().id >> c.sidebar.pages.back().title;
	}
	
	r=sql()<<
		"SELECT id,name "
		"FROM	cats";
	c.sidebar.cats.reserve(16);
	while(r.next()) {
		c.sidebar.cats.resize(c.sidebar.cats.size()+1);
		r >> c.sidebar.cats.back().id >> c.sidebar.cats.back().name;
	}

	r=sql()<<
		"SELECT link_cats.id,name,title,url,description "
		"FROM link_cats,links "
		"WHERE links.cat_id=link_cats.id "
		"ORDER BY link_cats.id";
	int previd = -1;
	while(r.next()) {
		int id;
		std::string gname,title,url,descr;
		r>>id>>gname>>title>>url>>descr;
		if(id!=previd) {
			c.sidebar.link_cats.push_back(data::blog::sidebar_info::link_cat());
			data::blog::sidebar_info::link_cat content;
			c.sidebar.link_cats.back().title=gname;
		}
		c.sidebar.link_cats.back().links.push_back(data::blog::sidebar_info::link_cat::link());
		data::blog::sidebar_info::link_cat::link &link = c.sidebar.link_cats.back().links.back();
		link.title = title;
		link.href = url;
		link.description = descr;
	}

	cache().store_data("sidebar_info",c.sidebar);
}

void master::prepare(data::blog::master &c)
{
	basic_master::prepare(c);
	sidebar_init ini = { this, &c };
	c.load_sidebar = ini;
}

void master::init()
{
	if(session().is_set("user")) {
		user_ = session().get("user");
	}
	else {
		user_.clear();
	}
}

void master::clear()
{
	user_.clear();
	basic_master::clear();
}


} // blog
} // apps
