#include <dbi/dbixx.h>
#include <iostream>

using namespace dbixx;
using namespace std;

void remove_tabs(string &s)
{
	unsigned i;
	for(i=0;i<s.size();i++){
		if(s[i]=='\t') s[i]=' ';
	}
}

void prepare_post(string &a,string &c)
{
	string tmp=c;
	size_t s;
	if((s=tmp.find("<!--more-->"))==string::npos) {
		a=tmp;
		c="";
	}
	else {
		a=tmp.substr(0,s);
		c=tmp.substr(s+11);
	}
	remove_tabs(a);
	remove_tabs(c);
}

int main()
{
	try {
		session wp("mysql");
		wp.param("dbname","wordpress");
		wp.param("username","root");
		wp.param("password","root");
		wp.connect();
		
		session sql("mysql");
		sql.param("dbname","newpress");
		sql.param("username","root");
		sql.param("password","root");

		/*
		session sql("sqlite3");
		sql.param("dbname","cppcms.db");
		sql.param("sqlite3_dbdir","./db/");
		*/

		sql.connect();

		transaction tr(sql);
		row r;
		string val;

		wp<<"select option_value from wp_options where option_name='blogname'";
		wp.single(r);
		r>>val;
		sql<<"insert into options values(0,?)",val,exec();

		wp<<"select option_value from wp_options where option_name='blogdescription'";
		wp.single(r);
		r>>val;
		sql<<"insert into options values(1,?)",val,exec();

		result res;

		wp<<"select ID,display_name from wp_users",res;
		while(res.next(r)) {
			int id;
			string name,pass;
			r>>id>>name;
			cout<<"Insert password for user "<<name<<":";
			cin>>pass;
			sql<<"insert into users(id,username,password) values(?,?,?)",
				id,name,pass,exec();
		}

		wp<<	"select ID,post_author,post_date,post_title,post_content,post_status,comment_count "
			"from wp_posts where post_type='post'";
		wp.fetch(res);
		while(res.next(r)) {
			int id;
			int uid,count;
			std::tm pub;
			string title;
			string content;
			string status;
			r>>id>>uid>>pub>>title>>content>>status>>count;
			string abstract;
			prepare_post(abstract,content);
			sql<<"Insert into posts(id,author_id,title,abstract,content,publish,is_open,comment_count) "
				"values(?,?,?,?,?,?,?,?)",
				id,uid,title,abstract,content,pub,(status=="publish" ? 1 : 0),count;
			sql.exec();
		}

		wp<<	"select ID,post_author,post_title,post_content "
			"from wp_posts where post_type='page'";
		wp.fetch(res);
		while(res.next(r)) {
			int uid,id;
			string title;
			string content;
			r>>id>>uid>>title>>content;
			remove_tabs(content);
			sql<<"Insert into pages(id,author_id,title,content,is_open) "
				"values(?,?,?,?,1)",id,uid,title,content;
			sql.exec();
		}

		

		wp<<"select comment_ID,comment_post_ID,comment_author,comment_author_email,"
			"comment_author_url,comment_date,comment_content from "
			"wp_comments where comment_approved=1;";
		wp.fetch(res);
		while(res.next(r)) {
			int id,pid;
			std::tm pub;
			string a,e,u,cont;
			r>>id>>pid>>a>>e>>u>>pub>>cont;
			remove_tabs(cont);
			sql<<"insert into comments(id,post_id,author,email,url,publish_time,content) "
				"values(?,?,?,?,?,?,?)",
				id,pid,a,e,u,pub,cont;
			sql.exec();
		}

		wp<<"select wp_terms.term_id,wp_terms.name from wp_term_taxonomy join wp_terms on wp_terms.term_id=wp_term_taxonomy.term_id where taxonomy='category'",res;
		wp.fetch(res);
		while(res.next(r)) {
			int id;
			string name;
			r>>id>>name;
			sql<<"insert into cats(id,name) values(?,?)",id,name,exec();
		}
		

		wp<<"select distinct wp_posts.ID,wp_posts.post_date,wp_terms.term_id,wp_posts.post_status from wp_term_taxonomy join wp_terms on wp_terms.term_id=wp_term_taxonomy.term_id left join wp_term_relationships on wp_term_taxonomy.term_taxonomy_id=wp_term_relationships.term_taxonomy_id join wp_posts on object_id=wp_posts.ID where taxonomy='category' and wp_posts.post_type='post' order by object_id";
		wp.fetch(res);
		while(res.next(r)) {
			int id;
			std::tm date;
			int tid;
			string status;
			r>>id>>date>>tid>>status;
			sql<<"insert into post2cat(cat_id,post_id,publish,is_open) values(?,?,?,?)",
				tid,id,date,(status=="publish" ? 1:0),exec();
		}
		
		wp<<"select distinct wp_posts.ID,wp_posts.post_date,wp_posts.post_status,wp_term_taxonomy.parent from wp_term_taxonomy join wp_terms on wp_terms.term_id=wp_term_taxonomy.term_id left join wp_term_relationships on wp_term_taxonomy.term_taxonomy_id=wp_term_relationships.term_taxonomy_id join wp_posts on object_id=wp_posts.ID where taxonomy='category' and wp_posts.post_type='post' order by object_id";
		wp.fetch(res);
		while(res.next(r)) {
			int id;
			std::tm date;
			int tid;
			string status;
			r>>id>>date>>status>>tid;
			sql<<"insert into post2cat(cat_id,post_id,publish,is_open) values(?,?,?,?)",
				tid,id,date,(status=="publish" ? 1:0),exec();
		}

		wp<<"select wp_terms.term_id,wp_terms.name from wp_term_taxonomy join wp_terms on wp_terms.term_id=wp_term_taxonomy.term_id where taxonomy='link_category'",res;
		wp.fetch(res);
		while(res.next(r)) {
			int id;
			string name;
			r>>id>>name;
			sql<<"insert into link_cats(id,name) values(?,?)",id,name,exec();
		}
		
		

		wp<<"select distinct wp_links.link_id,wp_links.link_url,wp_terms.term_id,wp_links.link_name,wp_links.link_description from wp_term_taxonomy join wp_terms on wp_terms.term_id=wp_term_taxonomy.term_id left join wp_term_relationships on wp_term_taxonomy.term_taxonomy_id=wp_term_relationships.term_taxonomy_id join wp_links on object_id=wp_links.link_id where taxonomy='link_category' order by object_id";
		wp.fetch(res);
		while(res.next(r)) {
			int id;
			int tid;
			string url,name,descr;
			r>>id>>url>>tid>>name>>descr;
			sql<<"insert into links(cat_id,id,title,url,description) values(?,?,?,?,?)",
				tid,id,name,url,descr,exec();
		}

		tr.commit();
	}
	catch(dbixx_error const &e){
		cerr<<e.what()<<endl;
	}

}
