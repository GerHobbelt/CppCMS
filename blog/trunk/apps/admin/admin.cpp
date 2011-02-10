#include <apps/admin/admin.h>
#include <apps/admin/summary.h>
#include <apps/admin/users.h>
#include <cppcms/url_mapper.h>
#include <cppcms/http_response.h>
#include <cppcms/session_interface.h>
namespace apps {
namespace admin {

admin_master::admin_master(cppcms::service &srv) : cppcms::application(srv) 
{
	mapper().assign("post","");
	mapper().assign("post","{1}");
	mapper().assign("page","");
	mapper().assign("page","{1}");
	mapper().assign("links","");
	mapper().assign("categories","");
	mapper().assign("options","");
	mapper().assign("cache","");
	mapper().assign("comment","{1}");

	attach( new users(srv),
		"users",
		"/users{1}",
		"/users(/?)",1);

	attach(	new summary(srv),
		"summary",
		"{1}",
		"/?",0);
}

void admin_master::main(std::string path)
{
	if(session().is_set("user") || path == "/users" || path == "/users/") {
		application::main(path);
	}
	else {
		response().set_redirect_header(url("/admin/users"));
	}
}

} // admin
} // apps
