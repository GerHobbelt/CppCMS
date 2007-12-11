#include "data.h"
#include "global_config.h"

using namespace std;

auto_ptr<Options> options;
auto_ptr<Comments> comments;
auto_ptr<Approved> approved;
auto_ptr<Posts> posts;
auto_ptr<Users> users;
auto_ptr<Texts_Collector> texts;
auto_ptr<Environment> env;


void db_initall()
{
	env=auto_ptr<Environment>(new Environment(global_config.sval( "bdb.path" ).c_str()));
	options=auto_ptr<Options>(new Options(*env));
	comments=auto_ptr<Comments>(new Comments(*env));
	approved=auto_ptr<Approved>(new Approved(*env));
	posts=auto_ptr<Posts>(new Posts(*env));
	users=auto_ptr<Users>(new Users(*env));
	texts=auto_ptr<Texts_Collector>(new Texts_Collector(*env,"texts.db"));
}

void db_openall()
{
	env->open();
	options->open();
	comments->open();
	approved->open();
	posts->open();
	users->open();
	texts->open();
}

void db_createall()
{
	env->create();
	options->create();
	comments->create();
	approved->create();
	posts->create();
	users->create();
	texts->create();
}

void db_closeall()
{
	options->close();
	comments->close();
	approved->close();
	posts->close();
	users->close();
	texts->close();
	env->close();
}


int db_configure()
{
	try {
		string username,password;
		db_initall();
		db_createall();
		cout<<"Blog owner username:";
		cin>>username;
		cout<<"Blog owner password:";
		cin>>password;
		user_t u;
		u.username=username.c_str();
		u.password=password.c_str();
		users->id.add(u);
		
		option_t op;
		op.id=option_t::BLOG_TITLE;
		op.value="CppBlog";
		options->insert(op);
		op.id=option_t::BLOG_DESCRIPTION;
		op.value="Yet another CppBlog";
		options->insert(op);
		db_closeall();
		return 0;
	}
	catch(DbException &e){
		cerr<<e.what();
		return 1;
	}
	catch(std::exception &e){
		cerr<<e.what();
		return 1;
	}
}

#ifdef CONFIG_ONLY
int main(int argc, char **argv)
{
	global_config.load(argc,argv);
	try {
		return db_configure();
	}
	catch(HTTP_Error &e) {
		cerr<<e.get();
	}
}
#endif
