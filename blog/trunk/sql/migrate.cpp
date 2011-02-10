#include <cppdb/frontend.h>
#include <iostream>


void copy(cppdb::session &from,cppdb::session &to)
{
	cppdb::result r;
	std::cout << "- checking versions" << std::endl;
	r = from << "SELECT value from text_options where id='dbversion'" << cppdb::row;
	if(r.empty() || r.get<std::string>(0)!="2") {
		throw std::runtime_error("Invalid source DB version");
	}
	r = to << "SELECT value from text_options where id='dbversion'" << cppdb::row;
	if(r.empty() || r.get<std::string>(0)!="2") {
		throw std::runtime_error("Invalid target DB version");
	}
	to << "DELETE from text_options where id='dbversion'" << cppdb::exec;
	std::cout << "- users" << std::endl;
	r = from << "SELECT id,username,password FROM users";
	while(r.next()) {
		to << "INSERT into users(id,username,password) values(?,?,?)" 
			<<r.get<int>(0)
			<< r.get<std::string>(1)
			<< r.get<std::string>(2)
			<< cppdb::exec;
	}
	std::cout << "- posts" << std::endl;
	r = from << "SELECT id,author_id,title,abstract,content,publish,is_open,comment_count from posts";
	while(r.next()) {
		to << "INSERT into posts(id,author_id,title,abstract,content,publish,is_open,comment_count) values(?,?,?,?,?,?,?,?)" 
			<<r.get<int>(0)
			<<r.get<int>(1)
			<< r.get<std::string>(2)
			<< r.get<std::string>(3)
			<< r.get<std::string>(4)
			<< r.get<std::tm>(5)
			<< r.get<int>(6)
			<< r.get<int>(7)
			<< cppdb::exec;
	}
	std::cout << "- comments" << std::endl;
	r = from << "SELECT id,post_id,author,email,url,publish_time,content from comments";
	while(r.next()) {
		to << "insert into comments values(?,?,?,?,?,?,?)"
			<<r.get<int>(0)
			<<r.get<int>(1)
			<< r.get<std::string>(2)
			<< r.get<std::string>(3)
			<< r.get<std::string>(4)
			<< r.get<std::tm>(5)
			<< r.get<std::string>(6)
			<< cppdb::exec;
	}
	std::cout << "- text_options" << std::endl;
	r = from << "SELECT id,value from text_options";
	while(r.next()) {
		to << "insert into text_options values(?,?)" 
			<< r.get<std::string>(0)
			<< r.get<std::string>(1)
			<< cppdb::exec;
	}
	std::cout << "- cats" << std::endl;
	r = from << "SELECT id,name from cats";
	while(r.next()) {
		to << "insert into cats values(?,?)" 
			<< r.get<int>(0)
			<< r.get<std::string>(1)
			<< cppdb::exec;
	}
	std::cout << "- post2cat" << std::endl;
	r = from << "SELECT post_id,cat_id,publish,is_open from post2cat";
	while(r.next()) {
		to << "insert into post2cat values(?,?,?,?)" 
			<< r.get<int>(0)
			<< r.get<int>(1)
			<< r.get<std::tm>(2)
			<< r.get<int>(3)
			<< cppdb::exec;
	}
	std::cout << "- link_cats" << std::endl;
	r = from << "SELECT id,name from link_cats";
	while(r.next()) {
		to << "insert into link_cats values(?,?)" 
			<< r.get<int>(0)
			<< r.get<std::string>(1)
			<< cppdb::exec;
	}
	std::cout << "- links" << std::endl;
	r = from << "SELECT id,cat_id,title,url,description from links";
	while(r.next()) {
		to << "insert into links values(?,?,?,?,?)" 
			<< r.get<int>(0)
			<< r.get<int>(1)
			<< r.get<std::string>(2)
			<< r.get<std::string>(3)
			<< r.get<std::string>(4)
			<< cppdb::exec;
	}
	std::cout << "- pages" << std::endl;
	r = from << "SELECT id,author_id,title,content,is_open from pages";
	while(r.next()) {
		to << "insert into pages values(?,?,?,?,?)" 
			<< r.get<int>(0)
			<< r.get<int>(1)
			<< r.get<std::string>(2)
			<< r.get<std::string>(3)
			<< r.get<int>(4)
			<< cppdb::exec;
	}
}

int main(int argc,char **argv)
{
	if(argc!=3) {
		std::cerr << "Usage src-connection-string tgt-connection-string" << std::endl;
		return 1;
	}
	try {
		cppdb::session from(argv[1]),to(argv[2]);
		cppdb::transaction trf(from);
		cppdb::transaction trt(to);

		copy(from,to);

		if(to.engine()=="postgresql") {
			to<<"select setval('users_id_seq',(select max(id) from users))"<<cppdb::row;
			to<<"select setval('posts_id_seq',(select max(id) from posts))"<<cppdb::row;
			to<<"select setval('comments_id_seq',(select max(id) from comments))"<<cppdb::row;
			to<<"select setval('pages_id_seq',(select max(id) from pages))"<<cppdb::row;
			to<<"select setval('links_id_seq',(select max(id) from links))"<<cppdb::row;
			to<<"select setval('cats_id_seq',(select max(id) from cats))"<<cppdb::row;
		}

		trt.commit();
		trf.commit();
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	std::cout << "Done" <<std::endl;
	return 0;
}
