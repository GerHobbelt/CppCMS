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

page_form::page_form(wiki *w):
	title("title",w->gettext("Title")),
	content("content",w->gettext("Content")),
	sidebar("sidebar",w->gettext("Sidebar")),
	save("save",w->gettext("Save")),
	save_cont("save_cont",w->gettext("Save and Continue")),
	preview("preview",w->gettext("Preview"))
{
	*this & title & content & sidebar & save & save_cont & preview;
	fields<<title<<content<<sidebar;
	buttons<<save<<save_cont<<preview;
	title.set_nonempty();
	content.set_nonempty();
	content.rows=30;
	content.cols=80;
	sidebar.rows=10;
	sidebar.cols=80;
}

}
