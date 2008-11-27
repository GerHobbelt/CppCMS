#include "mb.h"
#include "master_data.h"

namespace apps {

mb::mb(cppcms::worker_thread &w) :
	cppcms::application(w),
	forums(*this),
	thread(*this)
{
	sql.driver("mysql");
	sql.param("dbname",app.config.sval("mb.dbname"));
	sql.param("username",app.config.sval("mb.username"));
	sql.param("password",app.config.sval("mb.password"));
	sql.connect();
	use_template("simple");
}

void mb::main()
{
	try {
		application::main();
	}
	catch (e404 const &e){
		on_404();
	}
}

void mb::ini(data::master &c)
{
	c.main_page=env->getScriptName()+"/";
	c.media=app.config.sval("mb.media");
}


} // namespace apps
