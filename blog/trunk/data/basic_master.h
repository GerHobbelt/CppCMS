#pragma once
#include <cppcms/view.h>
#include <booster/function.h>
#include <cppcms/serialization.h>

namespace data {
	struct general_info : public cppcms::serializable {
		std::string blog_title;
		std::string blog_description;
		std::string contact;
		std::string copyright_string;

		void serialize(cppcms::archive &a)
		{
			a & blog_title & blog_description & contact & copyright_string;
		}
	};
	struct basic_master :public cppcms::base_content {
		general_info info;
		std::string media;
		std::string host;
		std::string cookie_prefix;
		booster::function<std::string(std::string const &)> markdown2html;
		booster::function<std::string(std::string const &)> xss;
	};
}


