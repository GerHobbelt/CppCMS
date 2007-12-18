#include "blog.h"
#include "templates/look.h"
#include <cppcms/templates.h>

#include <time.h>

void Blog::init()
{
	string root=global_config.sval("blog.script_path");

	fmt.media=global_config.sval("blog.media_path");

	url.add("^/?$",
		boost::bind(&Blog::main_page,this,"end"));
	fmt.main=root + "/";
	url.add("^/from/(\\d+)$",
		boost::bind(&Blog::main_page,this,$1));
	fmt.main_from=root + "/from/%1%";
	url.add("^/post/(\\d+)$",
		boost::bind(&Blog::post,this,$1));
	fmt.post=root + "/post/%1%";
	//url.add("^/admin$",
	//	boost::bind(&Blog::admin,this));
	fmt.admin=root + "/admin";
	//url.add("^/admin/new_post$",
	//	boost::bind(&Blog::new_post,this));
	fmt.new_post=root + "/admin/new_post";
	//url.add("^/admin/edit_post/(\\d+)$",
	//	boost::bind(&Blog::edit_post,this,$1));
	fmt.edit_post=root+"/adin/edit_post/%1%";
	//url.add("^/admin/edit_comment/(\\d+)$",
	//	boost::bind(&Blog::edit_comment,this,$1));
	//fmt.edit_comment=root+"/adin/edit_comment/%1%";
	// All incoming information
	//url.add("^/postback/comment/(\\d+)$",
	//	boost::bind(&Blog::add_comment,this));
	fmt.add_comment=root+"/postback/comment/%1%";
	//url.add("^/postback/post$",
	//	boost::bind(&Blog::add_post,this));
	fmt.add_post=root+"/postback/add_post";
	//url.add("^/postback/approve$",
	//	boost::bind(&Blog::approve,this));
	fmt.approve=root+"/postback/approve";

	// Default
	url.add("^.*$",boost::bind(&Blog::main_page,this,"end"));
}


void Blog::post(string s_id)
{
	int id=atoi(s_id.c_str());
	Content c(T_VAR_NUM);

	Renderer r(templates,TT_master,c);
	View_Main_Page view(this);
	view.ini_post(id);
	view.render(r,c,out.getstring());
}

void Blog::main_page(string from)
{

	Content c(T_VAR_NUM);

	Renderer r(templates,TT_master,c);
	View_Main_Page view(this);
	if(from=="end") {
		view.ini_main(-1);
	}
	else {
		view.ini_main(atoi(from.c_str()));
	}
	view.render(r,c,out.getstring());
}

void Blog::date(time_t time,string &d)
{
	struct tm t;
	localtime_r(&time,&t);
	char buf[80];
	snprintf(buf,80,"%02d/%02d/%04d, %02d:%02d",
		 t.tm_mday,t.tm_mon+1,t.tm_year+1900,
		 t.tm_hour,t.tm_min);
	d=buf;
}

void Blog::main()
{
	try {
		url.parse();
	}
	catch (DbException &err) {
		throw HTTP_Error(err.what());
	}
	catch(char const *s) {
		throw HTTP_Error(s);
	}

}
