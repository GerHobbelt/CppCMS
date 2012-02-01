///
/// Author: Artyom Beilis
///
///
#include <cppcms/application.h>
#include <cppcms/http_response.h>
#include <cppcms/url_mapper.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/json.h>
#include <cppdb/frontend.h>

#include <apps/config.h>
#include <data/config.h>
#include <apps/dbversion.h>

namespace apps {

config::config(cppcms::service &srv) : basic_master(srv)
{
	dispatcher().assign("/?",&config::configure,this);
	mapper().assign(""); // default empty url
	dispatcher().assign("/version",&config::version,this);
	mapper().assign("version","/version"); // default empty url
}

void config::version()
{
	data::config_version c;
	c.media = settings().get<std::string>("blog.media");
	c.version="1";
	c.expected = CPPBLOG_DBVERSION;
	cppdb::result r = sql() << "SELECT value FROM text_options WHERE id='dbversion'" << cppdb::row;
	if(!r.empty())
		r >> c.version;
	if(c.version == c.expected)
		response().set_redirect_header(url("/blog/summary"));
	else
		render("admin_skin","config_version",c);
}

void config::configure()
{
	data::config c;
	cppdb::result r = sql() << "SELECT value FROM text_options WHERE id='is_configured'" << cppdb::row;
	if(!r.empty() && r.get<std::string>(0)=="yes")
		response().set_redirect_header(url("/blog/summary"));
	if(request().request_method()=="POST") {
		c.form.load(context());
		if(c.form.validate()) {
			cppdb::transaction tr(sql());
			sql() << "DELETE FROM text_options WHERE id in ('blog_title','blog_description','is_configured','contact')" 
				<<cppdb::exec;
			sql()<<"INSERT INTO text_options values('is_configured','yes')" << cppdb::exec; 
			sql()<<"INSERT INTO text_options values('blog_title',?)" 
				<< c.form.title.value() << cppdb::exec;
			sql()<<"INSERT INTO text_options values('blog_description',?)" 
				<< c.form.subtitle.value() << cppdb::exec;
			sql()<<"INSERT INTO text_options values('contact',?)" 
				<< c.form.contact.value() << cppdb::exec;
			sql()<<"INSERT INTO users(username,password) values(?,?)" 
				<< c.form.username.value() << c.form.p1.value() << cppdb::exec;
			tr.commit();
			response().set_redirect_header(url("/blog/summary"));
			return;
		}
	}
	c.media = settings().get<std::string>("blog.media");
	render("admin_skin","config",c);
}

} // apps

