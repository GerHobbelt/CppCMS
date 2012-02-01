#include <apps/admin/options.h>
#include <data/admin/options.h>
#include <cppdb/frontend.h>

#include <cppcms/url_mapper.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
namespace apps {
namespace admin {


options::options(cppcms::service &s) : master(s)
{
	mapper().assign("/");
	dispatcher().assign("/?",&options::prepare,this);
}

void options::prepare()
{
	data::admin::options c;
	if(request().request_method()=="POST") {
		c.form.load(context());
		if(c.form.validate()) {
			cppdb::transaction tr(sql());
			sql()<<	"DELETE FROM text_options "
				"WHERE id in ('blog_title','blog_description','copyright','contact')"
				<< cppdb::exec;

			sql()<<"INSERT INTO text_options values('blog_title',?)" 
				<< c.form.title.value() << cppdb::exec;
			sql()<<"INSERT INTO text_options values('blog_description',?)" 
				<< c.form.subtitle.value() << cppdb::exec;
			sql()<<"INSERT INTO text_options values('contact',?)" 
				<< c.form.contact.value() << cppdb::exec;
			sql()<<"INSERT INTO text_options values('copyright',?)" 
				<< c.form.copyright.value() << cppdb::exec;
			cache().rise("options");
			tr.commit();
			load_form(c);
		}
		else {
			master::prepare(c);
		}
	}
	else {
		load_form(c);
	}
	render("admin_skin","options",c);
}

void options::load_form(data::admin::options &c)
{
	master::prepare(c);
	c.form.title.value(c.info.blog_title);
	c.form.subtitle.value(c.info.blog_description);
	c.form.contact.value(c.info.contact);
	c.form.copyright.value(c.info.copyright_string);
}

} // admin
} // apps
