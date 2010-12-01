#include "backend.h"
#include "driver_manager.h"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <stdlib.h>

extern "C" { 
	cppdb::backend::connection *cppdb_sqlite3_get_connection(); 
	cppdb::backend::connection *cppdb_postgres_get_connection();
}

using namespace std;


int last_line = 0;
int passed = 0;
int failed = 0;

#define TEST(x) do { last_line = __LINE__; if(x) { passed ++; break; } failed++; std::cerr<<"Failed in " << __LINE__ <<' '<< #x << std::endl; } while(0)


void test(std::string conn_str)
{
	cppdb::ref_ptr<cppdb::backend::connection> sql(cppdb::driver_manager::instance().connect(conn_str));
	cppdb::ref_ptr<cppdb::backend::statement> stmt;
	cppdb::ref_ptr<cppdb::backend::result> res;
	bool wide_api = (sql->driver()=="odbc" && conn_str.find("utf=wide")!=std::string::npos);
	
	bool pq_oid = false; // use translactions for blob
	
	if(sql->engine() == "postgresql") {
		if(sql->driver() == "odbc")
			pq_oid = true;
		else if(sql->driver()=="postgresql" && conn_str.find("@blob=bytea") == std::string::npos)
			pq_oid = true;
	}

	try {
		stmt = sql->prepare("drop table test");
		stmt->exec(); 
	}catch(...) {}

	if(sql->engine()=="mssql" && wide_api) {
		stmt = sql->prepare("create table test ( x integer not null, y nvarchar(1000) )");
	}
	else
		stmt = sql->prepare("create table test ( x integer not null, y varchar(1000) )");

	stmt->exec();
	stmt = sql->prepare("select * from test");
	res = stmt->query();
	TEST(!res->next());
	stmt  = sql->prepare("insert into test(x,y) values(10,'foo?')");
	stmt->exec();
	stmt = sql->prepare("select x,y from test");

	res = stmt->query();
	TEST(res->next());
	TEST(res->cols()==2);
	int iv;
	std::string sv;
	TEST(res->fetch(0,iv));
	TEST(iv==10);
	TEST(res->fetch(1,sv));
	TEST(sv=="foo?");
	TEST(!res->next());
	res.reset();
	stmt = sql->prepare("insert into test(x,y) values(20,NULL)");
	stmt->exec();
	stmt = sql->prepare("select y from test where x=?");
	stmt->bind(1,20);
	res = stmt->query();
	TEST(res->next());
	TEST(res->is_null(0));
	sv="xxx";
	TEST(!res->fetch(0,sv));
	TEST(sv=="xxx");
	TEST(!res->next());
	res.reset();
	stmt->reset();
	stmt->bind(1,10);
	res = stmt->query();
	TEST(res->next());
	sv="";
	TEST(!res->is_null(0));
	TEST(res->fetch(0,sv));
	TEST(sv=="foo?");
	stmt = sql->prepare("DELETE FROM test");
	stmt->exec();
	if(sql->engine()!="mssql" || wide_api) {
		stmt = sql->prepare("insert into test(x,y) values(?,?)");
		stmt->bind(1,15);
		stmt->bind(2,"שלום");
		stmt->exec();
		stmt = sql->prepare("select x,y from test");
		res = stmt->query();
		TEST(res->next());
		sv="";
		res->fetch(1,sv);
		TEST(sv=="שלום");
	}
	stmt = sql->prepare("drop table test");
	stmt->exec();
	stmt.reset();
	if(sql->engine() == "sqlite3") {
		stmt = sql->prepare("create table test ( id integer primary key autoincrement not null, n integer)");
	}
	else if(sql->engine() == "mysql") {
		stmt = sql->prepare("create table test ( id integer primary key auto_increment not null, n integer)");
	}
	else if(sql->engine() == "postgresql" )  {
		stmt = sql->prepare("create table test ( id  serial  primary key not null, n integer)");
	}
	else if(sql->engine() == "mssql" )  {
		stmt = sql->prepare("create table test ( id integer identity(1,1) primary key not null,n integer)");
	}
	if(stmt) {
		stmt->exec();
		stmt = sql->prepare("insert into test(n) values(?)");
		stmt->bind(1,10);
		stmt->exec();
		TEST(stmt->sequence_last("test_id_seq") == 1);
		stmt->reset();
		stmt->bind(1,20);
		stmt->exec();
		TEST(stmt->sequence_last("test_id_seq") == 2);
		stmt = sql->prepare("drop table test");
		stmt->exec();
		stmt.reset();
	}


	if(sql->engine() == "mssql") 
		stmt = sql->prepare("create table test ( i integer, r real, t datetime, s varchar(5000), bl varbinary(max))");
	else if(sql->engine() == "mysql") 
		stmt = sql->prepare("create table test ( i integer, r real, t datetime default null, s varchar(5000), bl blob) Engine = innodb");
	else if(sql->engine() == "postgresql") {
		if(pq_oid) {
			stmt = sql->prepare("create table test ( i integer, r real, t timestamp, s varchar(5000), bl oid)");
			stmt->exec();
			stmt = sql->prepare(	"CREATE TRIGGER t_test BEFORE UPDATE OR DELETE ON test "
						"FOR EACH ROW EXECUTE PROCEDURE lo_manage(bl)");
		}
		else {
			stmt = sql->prepare("create table test ( i integer, r real, t timestamp, s varchar(5000), bl bytea)");
		}
	}
	else
		stmt = sql->prepare("create table test ( i integer, r real, t timestamp, s varchar(5000), bl blob)");
	stmt->exec();
	if(sql->engine()=="mssql")
		stmt = sql->prepare("insert into test values(?,?,?,?,cast(? as varbinary(max)))");
	else
		stmt = sql->prepare("insert into test values(?,?,?,?,?)");
	stmt->bind_null(1);
	stmt->bind_null(2);
	stmt->bind_null(3);
	stmt->bind_null(4);
	stmt->bind_null(5);
	stmt->exec();
	TEST(stmt->affected()==1);
	if(pq_oid) {
		sql->begin();
	}
	stmt->reset();
	stmt->bind(1,10);
	stmt->bind(2,3.14);
	time_t now=time(0);
	std::tm t=*localtime(&now);
	stmt->bind(3,t);
	stmt->bind(4,"'to be' \\'or not' to be");
	std::istringstream iss;
	iss.str(std::string("\xFF\0\xFE\1\2",5));
	stmt->bind(5,iss);
	stmt->exec();
	if(pq_oid) {
		sql->commit();
		sql->begin();
	}
	stmt = sql->prepare("select i,r,t,s,bl from test");
	res = stmt->query();
	{
		TEST(res->cols()==5);
		TEST(res->column_to_name(0)=="i");
		TEST(res->column_to_name(1)=="r");
		TEST(res->column_to_name(2)=="t");
		TEST(res->column_to_name(3)=="s");
		TEST(res->column_to_name(4)=="bl");

		TEST(res->name_to_column("i")==0);
		TEST(res->name_to_column("r")==1);
		TEST(res->name_to_column("t")==2);
		TEST(res->name_to_column("s")==3);
		TEST(res->name_to_column("bl")==4);
		TEST(res->name_to_column("x")==-1);
		
		TEST(res->next());

		std::ostringstream oss;
		int i=-1; double r=-1; std::tm t=std::tm(); std::string s="def";
		TEST(res->is_null(0));
		TEST(res->is_null(1));
		TEST(res->is_null(2));
		TEST(res->is_null(3));
		TEST(res->is_null(4));
		TEST(!res->fetch(0,i));
		TEST(!res->fetch(1,r));
		TEST(!res->fetch(2,t));
		TEST(!res->fetch(3,s));
		TEST(!res->fetch(4,oss));
		TEST(i==-1 && r==-1 && t.tm_year == 0 && s=="def" && oss.str()=="");
		TEST(res->has_next() == cppdb::backend::result::next_row_unknown || res->has_next() == cppdb::backend::result::next_row_exists);
		TEST(res->next());
		TEST(res->fetch(0,i));
		TEST(res->fetch(1,r));
		TEST(res->fetch(2,t));
		TEST(res->fetch(3,s));
		TEST(res->fetch(4,oss));
		TEST(!res->is_null(0));
		TEST(!res->is_null(1));
		TEST(!res->is_null(2));
		TEST(!res->is_null(3));
		TEST(!res->is_null(4));
		TEST(i==10);
		TEST(3.1399 <= r && r <= 3.1401);
		TEST(mktime(&t)==now);
		TEST(s=="'to be' \\'or not' to be");
		TEST(oss.str() == std::string("\xFF\0\xFE\1\2",5));
		TEST(res->has_next() == cppdb::backend::result::next_row_unknown || res->has_next() == cppdb::backend::result::last_row_reached);
		TEST(!res->next());
	}
	if(pq_oid) {
		sql->commit();
	}
	stmt = sql->prepare("DELETE FROM test where 1<>0");
	stmt->exec();
	TEST(stmt->affected()==2);
	sql->begin();
	stmt = sql->prepare("insert into test(i) values(10)");
	stmt->exec();
	stmt = sql->prepare("insert into test(i) values(20)");
	stmt->exec();
	sql->commit();
	stmt = sql->prepare("select count(*) from test");
	res = stmt->query();
	TEST(res->next());
	TEST(res->fetch(0,iv) && iv==2);
	res.reset();
	stmt.reset();
	iv=-1;
	stmt = sql->prepare("DELETE FROM test where 1<>0");
	stmt->exec();
	sql->begin();
	stmt = sql->prepare("insert into test(i) values(10)");
	stmt->exec();
	stmt = sql->prepare("insert into test(i) values(20)");
	stmt->exec();
	sql->rollback();
	stmt = sql->prepare("select count(*) from test");
	res = stmt->query();
	TEST(res->next());
	iv=-1;
	TEST(res->fetch(0,iv));
	TEST(iv==0);
	stmt = sql->prepare("DELETE FROM test where 1<>0");
	stmt->exec();
	sql->begin();
	stmt = sql->prepare("insert into test(i,s) values(?,?)");
	int sizes[]={	0, // General 0 length string
			61,62,63,64,65,66,67, // mysql buffer size
			510,511,512,513,514, // ODBC buffer size in wchars
			1020,1021,1022,1023,1024,1025,1026,1027 // ODBC buffer size
		    };
	for(unsigned i=0;i<sizeof(sizes)/sizeof(int);i++) {
		int size = sizes[i];
		std::string value;
		value.reserve(size);
		srand(i);
		for(int j=0;j<size;j++) {
			value+=char(rand() % 26 + 'a');
		}
		stmt->bind(1,size);
		stmt->bind(2,value);
		stmt->exec();
		stmt->reset();
	}
	sql->commit();
	stmt = sql->prepare("select s from test where i=?");
	for(unsigned i=0;i<sizeof(sizes)/sizeof(int);i++) {
		int size = sizes[i];
		std::string value;
		value.reserve(size);
		srand(i);
		for(int j=0;j<size;j++) {
			value+=char(rand() % 26 + 'a');
		}
		stmt->bind(1,size);
		res = stmt->query();
		TEST(res->next());
		std::string v;
		TEST(res->fetch(0,v));
		TEST(v==value);
		res.reset();
		stmt->reset();
	}
	stmt = sql->prepare("DELETE FROM test where 1<>0");
	stmt->exec();

	sql->begin();
	if(sql->engine()=="mssql")
		stmt = sql->prepare("insert into test(i,bl) values(?,cast(? as varbinary(max)))");
	else
		stmt = sql->prepare("insert into test(i,bl) values(?,?)");
	for(unsigned i=0;i<sizeof(sizes)/sizeof(int);i++) {
		int size = sizes[i];
		std::stringstream value;
		srand(i);
		for(int j=0;j<size;j++) {
			value << char(rand() % 26 + 'a');
		}
		stmt->bind(1,size);
		stmt->bind(2,value);
		stmt->exec();
		stmt->reset();
	}
	sql->commit();

	stmt = sql->prepare("select bl from test where i=?");
	for(unsigned i=0;i<sizeof(sizes)/sizeof(int);i++) {
		int size = sizes[i];
		std::string value;
		value.reserve(size);
		srand(i);
		for(int j=0;j<size;j++) {
			value+=char(rand() % 26 + 'a');
		}
		if(pq_oid)
			sql->begin();
		stmt->bind(1,size);
		res = stmt->query();
		TEST(res->next());
		std::ostringstream v;
		TEST(res->fetch(0,v));
		TEST(v.str()==value);
		res.reset();
		stmt->reset();
		if(pq_oid)
			sql->commit();
	}
	stmt = sql->prepare("DELETE FROM test where 1<>0");
	stmt->exec();
	
}



int main(int argc,char **argv)
{
	if(argc!=2) {
		std::cerr << "Usage: test_backend connection_string" << std::endl;
		return 1;
	}
	std::string cs = argv[1];
	try {
		test(cs);
	}
	catch(std::exception const &e) {
		std::cerr << "Fail " << e.what() << std::endl;
		std::cerr << "Last tested line " << last_line  << std::endl;
		return 1;
	}
	std::cout << "Tests: " << passed+failed << " failed: " << failed << std::endl;
	if(failed > 0) {
		std::cout << "Fail!" << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
