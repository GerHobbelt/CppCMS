#include <apps/admin/caching.h>
#include <data/admin/caching.h>
#include <cppcms/http_request.h>
#include <cppcms/cache_interface.h>
#include <cppcms/url_dispatcher.h>

namespace apps {
namespace admin {
caching::caching(cppcms::service &s) : master(s)
{
	mapper().assign("");
	dispatcher().assign("/?",&caching::prepare,this);
}

void caching::prepare()
{
	if(request().request_method()=="POST")
		cache().clear();
	data::admin::caching c;
	unsigned keys = 0, triggers = 0;
	c.is_active = cache().stats(keys,triggers);
	c.size = keys;
	master::prepare(c);
	render("admin_skin","caching",c);
}


} // admin
} // cache
