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
	save("save",w->gettext("Save")),
	save_cont("save_cont",w->gettext("Save and Continue")),
	preview("preview",w->gettext("Preview"))
{
	*this & title & content & save & save_cont & preview;
	fields<<title<<content;
	buttons<<save<<save_cont<<preview;
	title.non_empty();
	content.non_empty();
}

}
