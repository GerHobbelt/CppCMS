#include "forums.h"
#include "forums_data.h"
#include "mb.h"
#include <boost/lexical_cast.hpp>
#include <cgicc/HTTPRedirectHeader.h>

using namespace dbixx;

namespace data {

new_topic_form::new_topic_form(cppcms::application &a) :
	title("title",a.gettext("Tittle")),
	author("author",a.gettext("Author")),
	comment("comment",a.gettext("Comment")),
	submit("submit",a.gettext("Create"))
{
	*this & title & author & comment & submit;
	title.set_nonempty();
	author.set_limits(1,64);
	comment.set_limits(1,256);
	comment.rows=15;
	comment.cols=40;
}

};

namespace apps {

forums::forums(mb &b) :
	application(b.worker),
	board(b)
{
	url.add("^(/(\\w+)?)?$",boost::bind(&forums::display_forums,this,_2));
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
	const unsigned topics_per_page=10;
	data::forums c(*this);
	board.ini(c);
	if(env->getRequestMethod()=="POST") {
		c.form.load(*cgi);
		if(c.form.validate()) {
			dbixx::transaction tr(board.sql);
			board.sql<<
				"INSERT INTO threads(title) VALUES(?)",
				c.form.title.get(),exec();
			int id=board.sql.rowid();
			board.sql<<
				"INSERT INTO messages(thread_id,reply_to,content,author) "
				"VALUES (?,0,?,?)",
				id,c.form.comment.get(),c.form.author.get(),exec();
			tr.commit();
			session["author"]=c.form.author.get();
			add_header("Status: 302 Found");
			set_header(new cgicc::HTTPRedirectHeader(board.thread.user_url(id)));
			return;
		}
	}
	int offset= page.empty() ? 0 : atoi(page.c_str());
	dbixx::result res;
	board.sql<<
		"SELECT id,title "
		"FROM threads "
		"ORDER BY id DESC "
		"LIMIT ?,?",offset*topics_per_page,topics_per_page,res;
	c.topics.resize(res.rows());
	dbixx::row r;
	for(int i=0;res.next(r);i++) {
		int id;
		r>>id>>c.topics[i].title;
		c.topics[i].url=board.thread.user_url(id);
	}
	if(c.topics.size()==topics_per_page) {
		c.next_page=forums_url(offset+1);
	}
	if(offset>0) {
		c.prev_page=forums_url(offset-1);
	}
	if(session.is_set("author")) {
		c.form.author.set(session["author"]);
	}
	render("forums",c);
}


} // namespace apps
