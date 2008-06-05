#include <dbi/dbixx.h>
#include <stdio.h>
#include <ctime>
#include <stdlib.h>
#include <iostream>

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
	string title="This is some long title";
	string content=readfile("content.txt");
	string comm_content=readfile("comm_content.txt");
	string abstract=readfile("abstract.txt");
	session sql;
	try{
/*		sql.driver("pgsql");
		sql.param("dbname","cppcms");
		sql.param("username","a.tonkikh");*/
		sql.driver("sqlite3");
		sql.param("dbname","cppcms.db");
		sql.param("sqlite3_dbdir","../db");
		sql.connect();

		sql<<"begin",exec();

		int i,j;
		char buffer[256];
		sql<<"insert into users values(1,'artik','artik')",exec();
		sql<<"insert into options values(0,'Blog title')",exec();
		sql<<"insert into options values(1,'Description')",exec();
		for(i=0;i<5;i++) {
			snprintf(buffer,sizeof(buffer),"page %d",i);
			sql<<"insert into pages(author_id,title,content,is_open) "
				"values(1,?,?,1)",buffer,(abstract+content),exec();
		}

		for(i=0;i<10;i++) {
			snprintf(buffer,sizeof(buffer),"category %d",i);
			sql<<"insert into cats(name) values(?)",buffer,exec();
			snprintf(buffer,sizeof(buffer),"group %d",i);
			sql<<"insert into link_cats(name) values(?)",buffer,exec();
			for(int j=0;j<3;j++) {
				snprintf(buffer,sizeof(buffer),"link %d",i*3+j);
				sql<<"insert into links(cat_id,title,url,description) "
					"values(?,?,'http://','')",i,buffer,exec();
			}
		}
		
		int P=atol(argc[1]);
		int C=atol(argc[2]);
		time_t tb;
		time(&tb);
		for(i=0;i<P;i++){
			std::tm t;
			time_t ct=tb+i*C;
			localtime_r(&ct,&t);
			sql<<	"insert into posts(id,author_id,title,"
				"abstract,content,publish,is_open,comment_count) "
				"values (?,?,?,?,?,?,?,?)",
				i,1,title,abstract,content,t,1,C,
				exec();
			for(j=i%2;j<10;j+=2) {
				sql<<"insert into post2cat values(?,?,?,?)",
					i,j,t,1,exec(); 
			}
			for(j=0;j<C;j++){
				ct=i*C+j;
				localtime_r(&ct,&t);
				sql<<	"insert into comments "
					"(post_id,author,email,url,content,"
					" publish_time) "
					"values(?,?,?,?,?,?)",
					i,"someone","mail@mail.ru","www.google.com",
					comm_content,t,
					exec();
			}
		}
		sql<<"commit",exec();
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
