#include "data.h"

namespace data {
edit_comment_form::edit_comment_form(worker_thread *w) :
		author("author",w->gettext("Author")),
		email("email",w->gettext("E-mail")),
		url("url",w->gettext("Url")),
		content("content"),
		save("save",w->gettext("Save")),
		del("delete",w->gettext("Delete")),
		sure("sure",w->gettext("I'm sure!"))
	{
		author.set_nonempty();
		email.set_nonempty();
		content.set_nonempty();
		content.rows=10;
		content.cols=80;
		*this & author & email & url & content & save & del & sure;
		fields<< author <<  email << url;
		buttons << save << del << sure ;
		sure.error_msg=w->gettext("Have you forgotten to check this?");
		email.help=w->gettext("(not displayed)");
	}
	bool edit_comment_form::validate()
	{
		if(form::validate()) {
			if(del.pressed && !sure.get()) {
				sure.not_valid();
				return false;
			}
			return true;
		}
		else {
			if(del.pressed && sure.get())
				return true;
			sure.not_valid();
			return false;
		}
	}

}
