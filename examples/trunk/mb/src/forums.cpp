#include "forums.h"
#include "forums_data.h"
#include "mb.h"
#include <boost/lexical_cast.hpp>

using namespace dbixx;

namespace data {

new_topic_form::new_topic_form(cppcms::application &a) :
	title("title",a.gettext("Tittle")),
	submit("submit",a.gettext("Create"))
{
	*this & title & submit;
	title.set_nonempty();
}

};

namespace apps {

forums::forums(mb &b) :
	application(b.worker),
	board(b)
{
	url.add("^(/(\\w+)?)?$",boost::bind(&forums::display_forums,this,$2));
}

string forums::forums_url(int offset)
{
	string link=env->getScriptName();
	if(offset==0)
		return link;
	link.append("/");
	link+=boost::lexical_cast<string>(offset);
	return link;
}

void forums::display_forums(string page)
{
	data::forums c(*this);
	board.ini(c);
	if(env->getRequestMethod()=="POST") {
		c.form.load(*cgi);
		if(c.form.validate()) {
			board.sql<<
				"INSERT INTO threads(title) VALUES(?)",
				c.form.title.get(),exec();
		}
		c.form.clear();
	}
	int offset= page.empty() ? 0 : atoi(page.c_str());
	dbixx::result res;
	board.sql<<
		"SELECT id,title "
		"FROM threads "
		"ORDER BY id DESC "
		"LIMIT ?,?",offset*10,10,res;
	c.topics.resize(res.rows());
	dbixx::row r;
	for(int i=0;res.next(r);i++) {
		int id;
		r>>id>>c.topics[i].title;
		c.topics[i].url=board.thread.tree_url(id);
	}
	if(c.topics.size()==10) {
		c.next_page=forums_url(offset+1);
	}
	render("forums",c);
}


} // namespace apps
