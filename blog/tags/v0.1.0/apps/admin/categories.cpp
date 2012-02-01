#include <apps/admin/categories.h>
#include <data/admin/categories.h>
#include <cppdb/frontend.h>

#include <cppcms/url_mapper.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
namespace apps {
namespace admin {

namespace {
	struct cats_form : public cppcms::form{
		cppcms::widgets::hidden id;
		cppcms::widgets::text cat;
		cppcms::widgets::submit save;
		cppcms::widgets::submit remove;
		cppcms::widgets::checkbox cascade;
		cats_form()
		{
			add(id);
			id.name("id");
			add(cat);
			cat.name("cat");
			cat.non_empty();
			add(save);
			save.name("save");
			add(remove);
			remove.name("remove");
			add(cascade);
			cascade.name("cascade");
			cascade.identification("1");
		}
	};
}


categories::categories(cppcms::service &s) : master(s)
{
	mapper().assign("/");
	dispatcher().assign("/?",&categories::prepare,this);
}

void categories::prepare()
{
	data::admin::categories c;
	if(request().request_method()=="POST") {
		cats_form form;
		form.load(context());
		if(!form.validate()) {
			if(!form.cat.valid()) {
				c.error_message = booster::locale::gettext("Invalid category name",context().locale());
			}
		}
		else {
			std::string sid = form.id.value();
			if(sid.empty()) {
				cppdb::transaction tr(sql());
				sql() << "INSERT INTO cats(name) VALUES(?)" << form.cat.value() << cppdb::exec;
				tr.commit();
				cache().rise("cats");
			}
			else {
				int id = atoi(sid.c_str());
				std::ostringstream ss;
				ss << "cat_" << id;
				std::string cat_key = ss.str();
				if(form.remove.value()) {
					cppdb::transaction tr(sql());
					cppdb::result r = sql() << 
						"SELECT post_id FROM post2cat WHERE cat_id = ? LIMIT 1" 
						<< id << cppdb::row;
					if(!r.empty()) {
						if(!form.cascade.value()) {
							c.error_message = 
								booster::locale::gettext("The category is not empty, can't delete",
											context().locale());
						}
						else {
							r.clear();
							sql() << "DELETE FROM post2cat WHERE cat_id=?" << id << cppdb::exec;
							sql() << "DELETE FROM cats WHERE id=?"<<id << cppdb::exec;
							tr.commit();
							cache().rise("cats");
							cache().rise(cat_key);
						}
					}
					else {
						sql() << "DELETE FROM cats WHERE id=?"<<id << cppdb::exec;
						tr.commit();
						cache().rise("cats");
						cache().rise(cat_key);
					}
				}
				else {
					cppdb::transaction tr(sql());
					sql() <<"UPDATE cats "
						"SET name=? "
						"WHERE id=? "<<form.cat.value() <<id << cppdb::exec;
					tr.commit();
					cache().rise("cats");
					cache().rise(cat_key);
				}
			}
		}
	}

	master::prepare(c);
	cppdb::result r;
	r = sql() << "SELECT id,name FROM cats";
	while(r.next()) {
		c.current_categories.resize(c.current_categories.size()+1);
		r >> c.current_categories.back().id >> c.current_categories.back().name;
	}
	
	render("admin_skin","categories",c);
}



} // admin
} // apps
