#include <apps/admin/users.h>
#include <data/admin/users.h>
#include <cppcms/url_mapper.h>
#include <cppcms/url_dispatcher.h>
#include <cppdb/frontend.h>
#include <cppcms/session_interface.h>
#include <cppcms/http_response.h>

namespace apps {
namespace admin {

users::users(cppcms::service &s) : master(s)
{
	mapper().assign("");
	dispatcher().assign("/?",&users::prepare,this);
}

void users::prepare()
{
	data::admin::users c;

	session().reset_session();
	session().clear();

	if(request().request_method()=="POST") {
		c.login.load(context());
		if(!c.login.validate()) {
			c.login.password.clear();
		}
		else {
			cppdb::result r;
			r = sql() << 
				"SELECT id,username,password "
				"FROM users "
				"WHERE username = ?" 
				<< c.login.username.value() << cppdb::row;
			if(!r.empty()) {
				int id;
				std::string user;
				std::string pass;
				r >> id >> user >> pass;
				if(c.login.username.value() == user && c.login.password.value() == pass) {
					session().set("user",user);
					session().set<int>("id",id);
					session().expose("user");
					response().set_redirect_header(url("/admin/summary"));
					return;
				}
			}
			c.login.password.clear();
			c.login.password.valid(false);
			c.login.username.valid(false);
		}
	}
	master::prepare(c);
	render("admin_skin","users",c);

}


} // admin
} // apps
