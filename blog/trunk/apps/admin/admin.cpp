#include <apps/admin/admin.h>
#include <cppcms/url_mapper.h>
namespace apps {
namespace admin {

admin_master::admin_master(cppcms::service &srv)
{
}
void admin_master::main(std::string url)
{
	if(!session().is_set("user")) {
	}
}

}
}
