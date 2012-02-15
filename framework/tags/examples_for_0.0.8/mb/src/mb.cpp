#include "mb.h"
#include "master_data.h"

namespace apps {

mb::mb(cppcms::worker_thread &w) :
	cppcms::application(w),
	forums(*this),
	thread(*this)
{
	dbixx_load(sql);
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
