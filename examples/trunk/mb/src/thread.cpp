#include "thread.h"
#include "thread_data.h"
#include "mb.h"
#include <cppcms/util.h>

using boost::lexical_cast;

namespace data {

reply_form::reply_form(application &a) :
	author("author",a.gettext("Author")),
	comment("comment",a.gettext("Comment")),
	send("send",a.gettext("Send"))
{
	*this & author & comment & send;
	author.set_limits(3,64);
	author.error_msg=a.gettext("Name too short or long");
	comment.set_limits(5,256);
	author.error_msg=a.gettext("Comment too short or long (at most 256 letters)");
}

reply::reply(application &a) : form(a)
{
}

string base_thread::text2html(string const &s)
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
		boost::bind(&thread::flat,this,$1));
	url.add("^/tree(\\d+)/?$",
		boost::bind(&thread::tree,this,$1));
}


string thread::flat_url(int id)
{
	return env->getScriptName()+"/flat/"+lexical_cast<string>(id);
}

string thread::tree_url(int id)
{
	return env->getScriptName()+"/tree/"+lexical_cast<string>(id);
}

string thread::reply_url(int thread_id,int message_id)
{
	string tmp=env->getScriptName()+"/comment/"+lexical_cast<string>(thread_id);
	if(message_id!=0) {
		tmp+="/to/"+lexical_cast<string>(message_id);
	}
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
	c.reply_to_thread=reply_url(id);
	return id;
}

void thread::flat(string sid)
{
	data::flat_thread c;
	int id=ini(sid,c);
	board.sql<<
		"SELECT id,author,content "
		"FROM threads WHERE thread_id=? "
		"ORDER BY id DESC",
		id;
	dbixx::result res;
	dbixx::row r;
	board.sql.fetch(res);
	c.messages.resize(res.rows());
	int i;
	for(i=0;res.next(r);i++) {
		int msg_id;
		r>>msg_id>>c.messages[i].author>>c.messages[id].content;
		c.messages[i].reply_url=reply_url(id,msg_id);
	}
	render("flat_thread",c);
}

namespace {

void make_tree(data::tree_thread::tree_msg::tree_t &messages,map<int,map<int,data::msg> > &data,int start)
{
	typedef map<int,map<int,data::msg> > all_t;
	std::pair<all_t::iterator,all_t::iterator>
		range=data.equal_range(start);
	for(all_t::iterator p=range.first;p!=range.second;++p) {
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
		"SELECT reply_id,id,author,content "
		"FROM threads WHERE thread_id=? "
		"ORDER BY reply_to,id DESC",
		id;
	dbixx::result res;
	dbixx::row r;
	board.sql.fetch(res);
	typedef map<int,map<int,data::msg> > all_t;
	all_t all;
	while(res.next(r)) {
		int msg_id,rpl_id;
		string author,comment;
		r>>rpl_id>>msg_id;
		data::msg &message=all[rpl_id][msg_id];
		r>>message.author>>message.content;
		message.reply_url=reply_url(id,msg_id);
	}
	
	make_tree(c.messages,all,0);

	render("tree_thread",c);
}

void thread::reply(string stid,string smid)
{
	int tid,mid;
	mid = smid.empty() ? 0 : lexical_cast<int>(smid);
	tid=lexical_cast<int>(stid);

	data::reply c(*this);

	if(env->getRequestMethod()=="POST") {
		c.form.load(*cgi);
		if(c.form.validate()) {
			dbixx:: transaction tr(board.sql);
			dbixx::row r;
			board.sql<<"SELECT count(*) FROM threads WHERE id=?",tid,r;
			int count;
			r>>count;
			if(count==0)
				throw e404();
			board.sql<<
				"INSERT INTO messages(reply_to,thread_id,author,content) "
				"VALUES(?,?,?,?) ",
				mid,tid,c.form.author.get(),c.form.comment.get();
			board.sql.exec();
		}
	}

	if(!smid.empty()) {
		mid=lexical_cast<int>(smid);
		tid=lexical_cast<int>(stid);
		board.sql<<
			"SELECT author,content,title "
			"FROM messages "
			"JOIN threads ON thread_id=threads.id "
			"WHERE messages.id=?",
			mid;
		dbixx::row r;
		if(!board.sql.single(r)) {
			throw e404();
		}
		r>>c.author>>c.content>>c.title;
	}
	else {
		mid=0;
		tid=ini(stid,c);
	}
}

} // namespace apps
