/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
 
#include "blog.h"
 
 
enum { 
	BLOG_MAIN,
	BLOG_POST,
	BLOG_CAT,
	BLOG_LOGIN,
	BLOG_LOGOUT,
	BLOG_POST_COMMENT,
	BLOG_ADMIN_MAIN,
	BLOG_ADMIN_COMMENTS_APPROVE,
	BLOG_ADMIN_COMMENT_EDIT,
	BLOG_ADMIN_POSTS_LIST,
	BLOG_ADMIN_POST_EDIT
};
 
void Blog::init()
{
	 // Using default global configuration 
	try {
		db.open();
	}
	catch (MySQL_DB_Err &err){
		throw HTTP_Error(err.get());
	}
	 
}

void Blog::authenticate_user()
{
	string username="";
	this->username="";
	string password="";
	
	user_id=-1;
	
	const vector<HTTPCookie> &cookies = env->getCookieList();
	
	for(i=0;i<cookies.size();i++) {
		if(cookies[i].getName()=="username") {
			username=cookies[i].getValue();
		}
		else if(cookies[i].getName()=="password") {
			password=cookies[i].getValue();
		}
	}
	if(username.empty()) {
		return;
	}
	MySQL_DB_Res res=db.query(escape(
			"SELECT id,cookie_password "
			"WHERE name=%1% "
			"LIMIT 1") << username);
	MySQL_DB_Row row;
	if((row=res.next())!=NULL) {
		if(password==row[1]) {// cookie_password
			user_id=atoi(row[0]);
			this->username=username;
		}
	}
}

void Blog::load_inputs()
{
	string s,tstr;
	form_iterator end=cgi->getElements.end();
	form_iterator f=cgi->getElement("p");
	if(f!=end){
		page=BLOG_POST;
		post_id=atoi(**f);
	}
	else{
		page=BLOG_MAIN;
	}
	
}

void show_post()
{
	char const q[]=
		"SELECT content_type,direction,title,abstract, "
		"       content,published,cb_users.name "
		"FROM cb_posts,cb_users "
		"WHERE cb_posts.id=%1% "
		"      AND cb_posts.state='published' "
		"      AND cb_posts.author_id=cb_users.id";
	MySQL_DB_Res res=db.query(escape(q)<<post_id);
	MySQL_DB_Row row;
	if((row=res.next())==NULL) {
		throw HTTP_Error("Not found",true);
	}
	bool is_html=(strcmp(row[0],"html")==0);
	bool is_rtl=(strcmp(row[1],"rtl")==0);
	char const *title=row[2];
	char const *abstract=row[3];
	string published=to_human_datetime(row[4]);
	char const *author_name=row[4];
	
	char const q2[] =
		"SELECT cb_categories.name, cat_id"
		"FROM cb_post2cat,cb_categories "
		"WHERE cb_post2cat.post_id=%1% "
		"      AND cb_post2cat.cat_id=cb_categories.id";
	
	MySQL_DB_Res res2=db.query(escape(q2)<<post_id);
	
	vector<string> cats;
	vector<int> cat_ids;
	
	cats.reserve(10);
	cat_ids.reserve(10);
	
	MySQL_DB_Row row2;
	
	while((row2=res2.next())!=NULL) {
		cats.push_back(row[0]);
		cat_ids.push_back(atoi(row[1]));
	}
	
	TEMPLATE_HEADER;
	TEMPLATE_POST_MAJOR;
	
	
	
	
}

void Blog::main()
{
	try{
		authenticate_user();
		load_inputs();
		switch (page) {
			case BLOG_MAIN:
				show_main_page();
				break;
			case BLOG_POST:
				show_post();
				break;
		}
	}
	catch(MySQL_DB_Err &e){
		throw HTTP_Error(string("SQL Error:")+e.get());
	}
}
