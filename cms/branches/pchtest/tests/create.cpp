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
	string title="This is some long title";
	string content=readfile("content.txt");
	string comm_content=readfile("comm_content.txt");
	string abstract=readfile("abstract.txt");
	session sql;
	try{
		sql.driver("pgsql");
		sql.param("dbname","cppcms");
		sql.param("username","a.tonkikh");
/*		sql.driver("sqlite3");
		sql.param("dbname","test.db");
		sql.param("sqlite3_dbdir","./");*/
		sql.connect();

		sql<<"begin",exec();
		

		sql<<"delete from comments",exec();
		sql<<"delete from options",exec();
		sql<<"delete from posts",exec();
		sql<<"delete from users",exec();
		sql<<"insert into users values(1,'artik','artik')",exec();
		sql<<"insert into options values(0,'CppBlog')",exec();
		sql<<"insert into options values(1,'Yet Another C++ Blog')",exec();
		int P=atol(argc[1]);
		int C=atol(argc[2]);
		int i,j;
		for(i=0;i<P;i++){
			std::tm t;
			time_t ct=i*C;
			localtime_r(&ct,&t);
			sql<<	"insert into posts(id,author_id,title,"
				"abstract,content,publish,is_open) "
				"values (?,?,?,?,?,?,?)",
				i,1,title,abstract,content,t,1,
				exec();
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
	return 0;
}
