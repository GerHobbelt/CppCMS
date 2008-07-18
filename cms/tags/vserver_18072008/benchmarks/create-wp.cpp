#include <dbi/dbixx.h>
#include <stdio.h>
#include <ctime>
#include <stdlib.h>
#include <iostream>
#include <list>

using namespace std;
using namespace dbixx;

string readfile(char const *n)
{
	string res;
	FILE *f=fopen(n,"r");
	if(!f) { perror(n); exit(1); };
	char buffer[1024];
	int num;
	while((num=fread(buffer,1,1024,f))>0) {
		buffer[num]=0;
		res+=buffer;
	}
	fclose(f);
	return res;
}

int main(int argv,char **argc)
{
	if(argv!=3) {
		cerr<<"Usage: POSTS COMMENTS-per-POST\n";
		return 1;
	}
	string title="לחיות עם גרסה יציבה של דביאן";
	string content=readfile("content.txt");
	string comm_content=readfile("comm_content.txt");
	string abstract=readfile("abstract.txt");
	session sql;
	try{
		sql.driver("mysql");
		sql.param("dbname","wordpress");
		sql.param("username","root");
		sql.param("password","root");
/*		sql.driver("pgsql");
		sql.param("dbname","cppcms");
		sql.param("username","a.tonkikh");*/
/*		sql.driver("sqlite3");
		sql.param("dbname","cppcms.db");
		sql.param("sqlite3_dbdir","../db");*/
		sql.connect();

		int i,j;
		char buffer[256];
		list<int> posts;

		
		list<int>::iterator p;
		time_t now;
		time(&now);

		int P=atol(argc[1]);
		int C=atol(argc[2]);

		for(i=0;i<P;i++) {
			int id=i+10;
			char guid[256];
			char name[256];
			snprintf(guid,sizeof(guid),"http://192.168.2.100/wp/?p=%d",id);
			snprintf(name,sizeof(name),"post-%d",id);
			now++;
			std::tm d1,d2;
			gmtime_r(&now,&d2);
			localtime_r(&now,&d1);
			sql<<
			"INSERT into wp_posts(ID,post_author,post_date,post_date_gmt,post_content,post_title,post_excerpt,post_password,post_name, "
			"to_ping,pinged,post_modified,post_modified_gmt,post_content_filtered,post_mime_type,guid) "
			"values(?,1,?,?,?,?,'','',?,'','',?,?,'','',?) ",
				id,d1,d2,abstract+"<!--more-->"+content,title,name,d1,d2,guid,exec();
			int ids[2][6] = { { 2,3,5,7,9,11} , {2,1,4,6,8,10} };
			for(j=0;j<6;j++){
				sql<<"insert into wp_term_relationships(object_id,term_taxonomy_id,term_order)"
					"values(?,?,0)",id,(ids[id % 2][j]),exec();
			}
			posts.push_back(id);
		}

		
		for(p=posts.begin();p!=posts.end();p++) {
			for(i=0;i<C;i++){
				now++;
				std::tm d1,d2;
				gmtime_r(&now,&d2);
				localtime_r(&now,&d1);
				sql<<"INSERT INTO wp_comments(comment_post_ID,comment_author,comment_author_email,comment_author_url,comment_author_IP,"
				"comment_date,comment_date_gmt,comment_content,comment_agent,comment_type) "
				"VALUES(?,'somone','somemail@mail.ru','http://somelink','192.168.2.100',?,?,?,'Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.8.1.13) Gecko/20080311 Firefox/2.0.0.13 (Debian-2.0.0.13-0etch1)','')",*p,d1,d2,comm_content,exec();

			}
			sql<<"update wp_posts set comment_count=(select count(*) from wp_comments where comment_post_ID=?) where ID=?",*p,*p,exec();
		}

	}
	catch(dbixx_error const &e){
		cerr<<e.what()<<endl;
	}
	catch(...)
	{
		cerr<<"Other error\n";
	}
	return 0;
}
