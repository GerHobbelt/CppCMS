#include "mb.h"
#include "master_data.h"

#include "forums.h"
#include "thread.h"
#include <cppcms/json.h>

namespace apps {

mb::mb(cppcms::service &w) :
	cppcms::application(w),
	forums(*this),
	thread(*this)
{
//	dbixx_load(sql);
}

void mb::ini(::data::master &c)
{
	c.main_page=request().script_name()+"/";
	c.media=settings().get<std::string>("mb.media");
}


} // namespace apps
