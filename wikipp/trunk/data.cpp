/* Nothing meanwhile */
#include "cxxmarkdown/markdowncxx.h"
#include "wiki.h"

namespace data {

string master::markdown(string s)
{
	string tmp;
	markdown2html(s,tmp);
	return tmp;
}

page_form::page_form(wiki *_w):
	w(_w),
	title("title",w->gettext("Title")),
	content("content",w->gettext("Content")),
	sidebar("sidebar",w->gettext("Sidebar")),
	save("save",w->gettext("Save")),
	save_cont("save_cont",w->gettext("Save and Continue")),
	preview("preview",w->gettext("Preview")),
	users_only("users_only",w->gettext("Only registered users may edit this page"))
{
	*this & title & content & sidebar & save & save_cont & preview & users_only;
	fields<<title<<content<<sidebar;
	buttons<<save<<save_cont<<preview<<users_only;
	users_only.error_msg=w->gettext("Only registered users may switch this state");
	title.set_nonempty();
	content.set_nonempty();
	content.rows=25;
	content.cols=60;
	sidebar.rows=10;
	sidebar.cols=60;
}

bool page_form::validate()
{
	bool res=form::validate();
	if(users_only.get() && !w->auth()) {
		users_only.not_valid();
		users_only.set(false);
		return false;
	}
	return res;
}

}
