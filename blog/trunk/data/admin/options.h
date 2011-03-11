#pragma once
#include <data/admin/master.h>

namespace data {
	namespace admin {
		struct options : public master {
			struct options_form : public cppcms::form {
				cppcms::widgets::text title;
				cppcms::widgets::text subtitle;
				cppcms::widgets::text contact;
				cppcms::widgets::textarea copyright;
				cppcms::widgets::submit save;
				options_form()
				{
					add(title);
					add(subtitle);
					add(contact);
					add(copyright);
					add(save);

					using booster::locale::translate;

					title.message(translate("Blog Title"));
					title.non_empty();
					subtitle.message(translate("Blog Subtitle"));
					subtitle.non_empty();
					contact.message(translate("Contact"));
					copyright.message(translate("Copyright Notice"));
					save.value(translate("Save"));
				}
			};
			options_form form;
		};
	} // admin
} // data

