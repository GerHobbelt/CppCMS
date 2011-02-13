#include <apps/admin/page.h>
#include <data/admin/page.h>
#include <cppdb/frontend.h>

#include <cppcms/url_mapper.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
#include <cppcms/session_interface.h>

namespace apps {
namespace admin {


page::page(cppcms::service &s) : master(s)
{
	mapper().assign("/{1}");
	dispatcher().assign("/(\\d+)/?",&page::prepare,this,1);
	mapper().assign("/");
	dispatcher().assign("/?",&page::prepare,this);
}

void page::prepare()
{
	prepare_shared();
}
void page::prepare(std::string sid)
{
	int id = atoi(sid.c_str());
	if(id==0) {
		response().make_error_response(404);
		return ;
	}
	prepare_shared(id);
}

void page::prepare_shared(int id)
{
	data::admin::page c;

	bool is_open = false;
	bool is_post = request().request_method()=="POST";

	if(id!=0){
		c.id = id;
		cppdb::result r;
		if(is_post) {
			r= sql() << 
				"SELECT is_open "
				"FROM pages "
				"WHERE id=?" << id << cppdb::row;
		}
		else {
			r=sql() <<
				"SELECT is_open,title,content "
				"FROM pages "
				"WHERE id=?" << id << cppdb::row;
		}
		if(r.empty()) {
			response().make_error_response(404);
			return;
		}
		is_open = r.get<int>(0);
		if(!is_post) {
			c.form.title.value(r.get<std::string>(1));
			c.form.content.value(r.get<std::string>(2));
		}
	}

	if(is_open)
		c.form.change_status.value(booster::locale::translate("Unpublish"));
	else
		c.form.change_status.value(booster::locale::translate("Publish"));

	if(is_post) {
		c.form.load(context());
		if(c.form.validate()) {
			if(id==0) {
				if(c.form.remove.value()) {
					response().set_redirect_header(url("/admin/summary"));
					return;
				}
				int open_status = 0;
				if(c.form.change_status.value())
					open_status = 1;

				cppdb::statement st;
				st = sql() << 
					"INSERT INTO pages(author_id,title,content,is_open) "
					"VALUES(?,?,?,?)" 
					<< session().get<int>("id")
					<< c.form.title.value()
					<< c.form.content.value()
					<< open_status << cppdb::exec;
				if(open_status || c.form.change_status.value())
					cache().rise("pages");
				id = st.sequence_last("page_id_seq");
				if(open_status)
					response().set_redirect_header(url("/blog/page",id));
				else if(c.form.save_and_continue.value())
					response().set_redirect_header(url("/admin/page",id));
				else
					response().set_redirect_header(url("/admin/summary"));
				return;

			}
			else {
				if(c.form.remove.value()) {
					sql() << "DELETE FROM pages where id=?" << id <<cppdb::exec;
					if(is_open) {
						std::ostringstream ss;
						ss << "page_" << id;
						cache().rise(ss.str());
						cache().rise("pages");
					}
					response().set_redirect_header(url("/admin/summary"));
					return;
				}
				int open_status = is_open ? 1 : 0;
				if(c.form.change_status.value())
					open_status = open_status ^ 1;
				sql()<< "UPDATE pages "
					"SET title=?,content=?,is_open=? "
					"WHERE id=?" 
					<< c.form.title.value()
					<< c.form.content.value()
					<< open_status 
					<< id << cppdb::exec;
				if(open_status || c.form.change_status.value()) {
					std::ostringstream ss;
					ss << "page_" << id;
					cache().rise(ss.str());
					cache().rise("pages");
				}
				if(c.form.change_status.value() || c.form.save.value()) {
					if(!open_status) 
						response().set_redirect_header(url("/admin/summary"));
					else 
						response().set_redirect_header(url("/blog/page",id));
					return;
				}

			}  // else if id
		} // if valid
	} // if post
	master::prepare(c);
	render("admin_skin","page",c);
}



} // admin
} // apps
