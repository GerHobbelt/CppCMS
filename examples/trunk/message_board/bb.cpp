#include <cppcms/worker_thread.h>
#include <cppcms/manager.h>
#include <tmpl/content.h>
#include <tmpl/renderer.h>
#include <dbixx/dbixx.h>
#include <iostream>
#include <boost/format.hpp>
#include <cgicc/HTTPRedirectHeader.h>
#include <cgicc/HTTPStatusHeader.h>

using namespace std;
using namespace cppcms;
using namespace dbixx;
using namespace tmpl;
using cgicc::HTTPRedirectHeader;
using cgicc::HTTPStatusHeader;
using boost::format;


void to_text(string const &in,string &out)
{
   int i;
   for(i=0;i<in.size();i++) {
     if(in[i]=='\n'){
       out+="<br />\n";
     }
     else {
       out+=in[i];
     }
   }
}

class bb : public worker_thread {
    renderer rnd;
    session sql;
    content cnt;
    // Major functions
    void main_page(string from);
    void topic(string no);
    void new_topic();
    // Major URLS
    string base_url;
    string more_url;
    string topic_url;
    string new_topic_url;
public:
    bb(manager const &s,template_data const &t);
    virtual void main();
};

bb::bb(manager const &s,template_data const &t):
        worker_thread(s),
	rnd(t)
{
    sql.driver("sqlite3");
    sql.param("sqlite3_dbdir",app.config.sval("sql.dir"));
    sql.param("dbname",app.config.sval("sql.db"));
    sql.connect();
        
    base_url=app.config.sval("bb.base_url");

    url.add("^/?$",
        boost::bind(&bb::main_page,this,"0"));

    url.add("^/page/(\\d+)$",
        boost::bind(&bb::main_page,this,$1));
    more_url=base_url+"/page/%d";

    url.add("^/topic/(\\d+)$",
        boost::bind(&bb::topic,this,$1));
    topic_url=base_url+"/topic/%d";

    url.add("^/new_topic$",
        boost::bind(&bb::new_topic,this));
    new_topic_url=base_url+"/new_topic";
    
    rnd.add_string_filter("totext",
       boost::bind(to_text,_1,_2));
};

void bb::new_topic()
{
    cgicc::const_form_iterator t=cgi->getElement("title");
    if(t==cgi->getElements().end()
      || t->getValue()=="") 
    {
       set_header(new HTTPRedirectHeader(base_url));
       return;
    }
    string title=t->getValue();
    sql<<"insert into topics(title) values(?) ",title;
    sql.exec();
    int id=sql.rowid();
    string redirect_url=str(format(topic_url) % id);
    set_header(new HTTPRedirectHeader(redirect_url));
}

void bb::topic(string id)
{
    transaction tr(sql);
    int topic_id=atoi(id.c_str());
    sql<<"SELECT title FROM topics WHERE id==?",topic_id;
    row r;
    if(!sql.single(r)) {
        set_header(new HTTPRedirectHeader(base_url));
	return;
    }
    string title;
    r>>title;
    cgicc::const_form_iterator t,m;
    t=cgi->getElement("title");
    m=cgi->getElement("content");
    if(t!=cgi->getElements().end() 
       && m!=cgi->getElements().end()
       && t->getValue()!="")
    {
        sql<<"insert into messages(topic_id,title,content) "
             "values(?,?,?)",
                topic_id,t->getValue(),m->getValue();
	sql.exec();
    }
    result res;
    cnt["title"]=title;
    sql<<"SELECT title,content FROM messages "
         "WHERE topic_id=? "
	 "ORDER BY id",topic_id,res;
    content::vector_t &v=cnt.vector("messages",res.rows());
    int i;
    for(i=0;res.next(r);i++){
	string title,content;
	r>>title>>content;
	v[i]["title"]=title;
	v[i]["content"]=content;
    }
    tr.commit();
    cnt["submit_url"]=str(format(topic_url) % topic_id);
    cnt["base_url"]=base_url;
    cnt["master_ref"]=string("topic");
    rnd.render(cnt,"master",out);
}

void bb::main_page(string from)
{
    int id,i;
    string title;
    int page=atol(from.c_str());
     
    result res;
    sql<<"select id,title from topics "
         "order by id desc "
         "limit 11 offset ?",page*10,res;
    int msg=res.rows();
    if(msg>10) msg=10;
    row r;
    content::vector_t &v=
        cnt.vector("topics",msg);

    for(i=0;i<10 && res.next(r);i++) {
	r>>id>>title;
	v[i]["url"]=
           str(format(topic_url) % id);
	v[i]["title"]=title;
    }
    if(res.next(r)) {
        cnt["more_url"]=
	   str(format(more_url) % (page+1));
    }
    cnt["submit_url"]=new_topic_url;

    cnt["master_ref"]=string("main");
    rnd.render(cnt,"master",out);
}

void bb::main()
{
    cnt.clear();
    if(url.parse()==-1) {
       set_header(new HTTPStatusHeader(404,"not found"));
    }
}

int main(int argc,char ** argv)
{
    template_data templ;
    try {
        manager app(argc,argv);
        templ.load(app.config.sval("templates.file"));
	app.set_worker(
	    new one_param_factory<bb,template_data>(templ));
	app.execute();
    }
    catch(std::exception const &e) {
        cerr<<e.what()<<endl;
    }
}

