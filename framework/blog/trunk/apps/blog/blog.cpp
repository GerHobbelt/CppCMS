#include <apps/blog/blog.h>
#include <apps/blog/static_page.h>
#include <apps/blog/post.h>
#include <apps/blog/summary.h>

namespace apps {
namespace blog {

blog_master::blog_master(cppcms::service &s) : cppcms::application(s)
{
	attach(	new static_page(s),
		"static_page",
		"/page{1}",
		"/page((/.*)?)",1);

	attach(	new post(s),
		"post",
		"/post{1}",
		"/post((/.*)?)",1);

	attach(	new summary(s),
		"summary",
		"{1}",
		".*",0);
}

} // blog
} // apps
