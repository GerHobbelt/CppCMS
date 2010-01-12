#include "thread.h"
#include "thread_data.h"
#include "mb.h"
#include <cppcms/util.h>
#include <cgicc/HTTPRedirectHeader.h>

using boost::lexical_cast;

namespace data {

reply_form::reply_form(application &a) :
	author("author",a.gettext("Author")),
	comment("comment",a.gettext("Comment")),
	send("send",a.gettext("Send"))
{
	*this & author & comment & send;
	author.set_limits(1,64);
	comment.set_limits(1,256);
	comment.rows=15;
	comment.cols=40;
}

reply::reply(application &a) : form(a)
{
}

string thread_shared::text2html(string const &s)
{
	string tmp=cppcms::escape(s);
	string res;
	res.reserve(tmp.size());
	for(unsigned i=0;i<tmp.size();i++) {
		if(tmp[i]=='\n') {
			res+="<br />";
		}
		res+=tmp[i];
	}
	return res;
}


} // namespace data

namespace apps {

thread::thread(mb &b) : application(b.worker) , board(b) 
{
	url.add("^/flat/(\\d+)/?$",
		boost::bind(&thread::flat,this,_1));
	url.add("^/tree/(\\d+)/?$",
		boost::bind(&thread::tree,this,_1));
	url.add("^/comment/(\\d+)?$",
		boost::bind(&thread::reply,this,_1));
}

string thread::flat_url(int id)
{
	return env->getScriptName()+"/flat/"+lexical_cast<string>(id);
}

string thread::tree_url(int id)
{
	return env->getScriptName()+"/tree/"+lexical_cast<string>(id);
}

string thread::user_url(int id)
{
	if(!session.is_set("view") || session["view"]=="tree") {
		return tree_url(id);
	}
	return flat_url(id);
}

string thread::reply_url(int message_id)
{
	string tmp=env->getScriptName();
	tmp+="/comment/";
	tmp+=lexical_cast<string>(message_id);
	return tmp;
}
int thread::ini(string sid,data::base_thread &c)
{
	int id=lexical_cast<int>(sid);
	board.sql<<"SELECT title FROM threads WHERE id=?",id;
	dbixx::row r;
	if(!board.sql.single(r)) {
		throw e404();
	}
	board.ini(c);
	r>>c.title;
	c.flat_view=flat_url(id);
	c.tree_view=tree_url(id);
	return id;
}

void thread::flat(string sid)
{
	data::flat_thread c;
	int id=ini(sid,c);
	board.sql<<
		"SELECT id,author,content "
		"FROM messages WHERE thread_id=? "
		"ORDER BY id",
		id;
	dbixx::result res;
	dbixx::row r;
	board.sql.fetch(res);
	c.messages.resize(res.rows());
	int i;
	for(i=0;res.next(r);i++) {
		int msg_id;
		r>>msg_id>>c.messages[i].author>>c.messages[i].content;
		c.messages[i].reply_url=reply_url(msg_id);
	}
	session["view"]="flat";
	render("flat_thread",c);
}

typedef map<int,map<int,data::msg> > msg_ord_t;

namespace {

void make_tree(data::tree_t &messages,map<int,map<int,data::msg> > &data,int start)
{
	std::pair<msg_ord_t::iterator,msg_ord_t::iterator>
		range=data.equal_range(start);
	for(msg_ord_t::iterator p=range.first;p!=range.second;++p) {
		for(map<int,data::msg>::iterator p2=p->second.begin(),e=p->second.end();p2!=e;++p2) {
			data::tree_thread::tree_msg &m=messages[p2->first];
			m.author=p2->second.author;
			m.content=p2->second.content;
			m.reply_url=p2->second.reply_url;
			make_tree(m.repl,data,p2->first);
		}
	}
	
}

}

void thread::tree(string sid)
{
	data::tree_thread c;
	int id=ini(sid,c);
	board.sql<<
		"SELECT reply_to,id,author,content "
		"FROM messages WHERE thread_id=? "
		"ORDER BY reply_to,id DESC",
		id;
	dbixx::result res;
	dbixx::row r;
	board.sql.fetch(res);
	msg_ord_t all;
	while(res.next(r)) {
		int msg_id,rpl_id;
		string author,comment;
		r>>rpl_id>>msg_id;
		data::msg &message=all[rpl_id][msg_id];
		r>>message.author>>message.content;
		message.reply_url=reply_url(msg_id);
	}
	
	make_tree(c.messages,all,0);

	session["view"]="tree";
	render("tree_thread",c);
}

void thread::reply(string smid)
{
	int mid;
	mid=lexical_cast<int>(smid);

	data::reply c(*this);

	if(env->getRequestMethod()=="POST") {
		c.form.load(*cgi);
		if(c.form.validate()) {
			dbixx::transaction tr(board.sql);
			dbixx::row r;
			board.sql<<"SELECT thread_id FROM messages WHERE id=?",mid;
			if(!board.sql.single(r))
				throw e404();
			int tid;
			r>>tid;
			board.sql<<
				"INSERT INTO messages(reply_to,thread_id,author,content) "
				"VALUES(?,?,?,?) ",
				mid,tid,c.form.author.get(),c.form.comment.get();
			board.sql.exec();
			tr.commit();

			session["author"]=c.form.author.get();

			add_header("Status: 302 Found");
			set_header(new cgicc::HTTPRedirectHeader(user_url(tid)));
			return;
		}
	}

	board.ini(c);
	if(session.is_set("author")) {
		c.form.author.set(session["author"]);
	}
	dbixx::row r;
	board.sql<<
		"SELECT threads.id,author,content,title "
		"FROM messages "
		"JOIN threads ON thread_id=threads.id "
		"WHERE messages.id=?",
		mid;
	if(!board.sql.single(r)) {
		throw e404();
	}

	int tid;

	r>>tid>>c.author>>c.content>>c.title;

	c.back=user_url(tid);

	render("reply",c);
}

} // namespace apps
