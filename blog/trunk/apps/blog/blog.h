#pragma once
#include <cppcms/application.h>
namespace apps {
	namespace blog {
		class blog_master : public cppcms::application {
		public:
			blog_master(cppcms::service &srv);
		};
	}
}
