#pragma once

#include <cppcms/application.h>

namespace apps {
	class captcha : public cppcms::application {
	public:
		captcha(cppcms::service &srv);
		void main(std::string url);
	};
}
