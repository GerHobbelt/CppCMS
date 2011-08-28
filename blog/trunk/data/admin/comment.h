#pragma once
#include <data/admin/master.h>
#include <cppcms/xss.h>

namespace data {
	namespace admin {
		struct comment : public master {
			struct comment_form : public cppcms::form {
				cppcms::widgets::text author;
				cppcms::widgets::text mail;
				cppcms::widgets::text url;
				cppcms::widgets::textarea content;
				cppcms::widgets::submit save;
				cppcms::widgets::submit erase;
				cppcms::widgets::checkbox confirm;
				
				cppcms::form data;
				cppcms::form buttons;

				comment_form()
				{
					add(data);
					add(buttons);

					data.add(author);
					data.add(url);
					data.add(mail);
					data.add(content);
					buttons.add(save);
					buttons.add(erase);
					buttons.add(confirm);

					using booster::locale::translate;


					author.non_empty();
					author.limits(1,256);
					author.message(translate("Author"));
					mail.message(translate("E-Mail"));
					url.message(translate("URL"));
					content.message(translate("Content"));
					content.limits(1,32768);
					save.value(translate("Save"));
					erase.value(translate("Delete"));
					confirm.message(translate("Confirm Deletion"));
					confirm.error_message(translate("Check to Delete"));
				}
				virtual bool validate()
				{
					if(!cppcms::form::validate())
						return false;
					if(erase.value() && !confirm.value()) {
						confirm.valid(false);
						return false;
					}
					else 
						return true;
				}
			};
			comment_form form;
			int id;
			int post_id;

			comment() : id(0), post_id(0) {}
		};
	} // admin
} // data

