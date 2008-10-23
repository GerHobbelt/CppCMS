#include "data.h"

namespace data {
edit_comment_form::edit_comment_form(worker_thread *w) :
		author("author",w->gettext("Author")),
		email("email",w->gettext("E-mail")),
		url("url",w->gettext("Url")),
		content("content"),
		save("save",w->gettext("Save")),
		del("delete",w->gettext("Delete"))
	{
		author.set_nonempty();
		email.set_nonempty();
		content.set_nonempty();
		content.rows=24;
		content.cols=80;
		*this & author & email & url & save & del & content;
		email.help=w->gettext("(not displayed)");
	}

}
