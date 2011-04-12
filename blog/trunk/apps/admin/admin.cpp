#include <apps/admin/admin.h>
#include <apps/admin/summary.h>
#include <apps/admin/users.h>
#include <apps/admin/page.h>
#include <apps/admin/post.h>
#include <apps/admin/caching.h>
#include <apps/admin/options.h>
#include <apps/admin/categories.h>
#include <apps/admin/comment.h>
#include <cppcms/url_mapper.h>
#include <cppcms/http_response.h>
#include <cppcms/session_interface.h>
namespace apps {
namespace admin {

admin_master::admin_master(cppcms::service &srv) : cppcms::application(srv) 
{
	attach( new caching(srv),
		"caching",
		"/cache{1}",
		"/cache((/.*)?)",1);
	
	attach(	new categories(srv),
		"categories",
		"/categories{1}",
		"/categories((/.*)?)",1);

	attach(	new options(srv),
		"options",
		"/options{1}",
		"/options((/.*)?)",1);

	attach(	new comment(srv),
		"comment",
		"/comment{1}",
		"/comment((/.*)?)",1);

	attach( new post(srv),
		"post",
		"/post{1}",
		"/post((/.*)?)",1);

	attach( new page(srv),
		"page",
		"/page{1}",
		"/page((/.*)?)",1);

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
