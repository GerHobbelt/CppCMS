#include <apps/blog/blog.h>
#include <apps/feed/feed.h>

#include <cppcms/service.h>
#include <cppcms/applications_pool.h>
#include <cppcms/url_mapper.h>
#include <cppcms/json.h>

#include <iostream>


class blog : public cppcms::application{
public:
	blog(cppcms::service &s) : cppcms::application(s)
	{
		attach( new apps::feed::feed_master(s),
			"feed",
			"/rss{1}",
			"/rss((/.*)?)",1);

		attach( new apps::blog::blog_master(s),
			"blog",
			"{1}",
			"((/.*)?)",1);

		mapper().root(settings().get<std::string>("blog.root"));
	}
};


int main(int argc,char **argv)
{
	try {
		cppcms::service srv(argc,argv);
		srv.applications_pool().mount(cppcms::applications_factory<blog>());
		srv.run();
	}
	catch(std::exception const &e) {
		std::cerr << "Failed: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
