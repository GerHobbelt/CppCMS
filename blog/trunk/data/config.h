#pragma once
#include <cppcms/view.h>

namespace data {
	struct  base_config : public cppcms::base_content {
		std::string media;
	};
	struct config_version  : public base_config {
		std::string version,expected;
	};
	struct config : public base_config {
		struct config_form : public cppcms::form {

			cppcms::widgets::text title;
			cppcms::widgets::text subtitle;
			cppcms::widgets::text contact;
			cppcms::widgets::text username;
			cppcms::widgets::password p1,p2;
			cppcms::widgets::submit save;

			config_form()
			{
				add(title);
				add(subtitle);
				add(contact);
				add(username);
				add(p1);
				add(p2);
				add(save);
				
				using booster::locale::translate;

				title.message(translate("Blog Title"));
				title.non_empty();
				subtitle.message(translate("Blog Subtitle"));
				subtitle.non_empty();
				contact.message(translate("Contact"));
				username.message(translate("Username"));
				username.non_empty();
				p1.message(translate("Password"));
				p1.non_empty();
				p2.message(translate("Confirm Password"));
				p2.non_empty();
				p1.check_equal(p2);
				save.value(translate("Save"));
			}
		};

		config_form form;
	};
}
