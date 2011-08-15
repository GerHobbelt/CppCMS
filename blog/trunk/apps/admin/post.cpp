#include <apps/admin/post.h>
#include <data/admin/post.h>
#include <cppdb/frontend.h>

#include <cppcms/url_mapper.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
#include <cppcms/session_interface.h>
#include <cppcms/cache_interface.h>
#include <booster/posix_time.h>

namespace apps {
namespace admin {


post::post(cppcms::service &s) : master(s)
{
	mapper().assign("/{1}");
	dispatcher().assign("/(\\d+)/?",&post::prepare,this,1);
	mapper().assign("/");
	dispatcher().assign("/?",&post::prepare,this);
}

void post::prepare()
{
	prepare_shared();
}
void post::prepare(std::string sid)
{
	int id = atoi(sid.c_str());
	if(id==0) {
		response().make_error_response(404);
		return ;
	}
	prepare_shared(id);
}

void post::prepare_shared(int id)
{
	data::admin::post c;

	bool is_open = false;
	bool is_post = request().request_method()=="POST";
	std::set<int> in_use_set;
	cppdb::result r;

	if(id!=0){
		c.id = id;
		if(is_post) {
			r= sql() << 
				"SELECT is_open "
				"FROM posts "
				"WHERE id=?" << id << cppdb::row;
		}
		else {
			r=sql() <<
				"SELECT is_open,title,abstract,content "
				"FROM posts "
				"WHERE id=?" << id << cppdb::row;

		}

		if(r.empty()) {
			response().make_error_response(404);
			return;
		}

		is_open = r.get<int>(0);

		if(!is_post) {
			c.form.title.value(r.get<std::string>(1));
			c.form.abstract.value(r.get<std::string>(2));
			c.form.content.value(r.get<std::string>(3));
		}
			
		// get all categories the post belong to
		r = sql() <<
			"SELECT post2cat.cat_id,cats.name "
			"FROM	post2cat "
			"JOIN	cats on cats.id = post2cat.cat_id "
			"WHERE	post2cat.post_id = ?" << id;
		while(r.next()) {
			std::string cat_id;
			std::string name;
			r >> cat_id >> name;
			cppcms::widgets::checkbox *cb = new cppcms::widgets::checkbox();
			c.form.del_from_cat.attach(cb);
			cb->help(name);
			cb->identification(cat_id);
			in_use_set.insert(atoi(cat_id.c_str()));
			c.form.del_from_cat_list.push_back(cb);
		}
	}
	/// Set of all categories the post not in then
	r = sql() << "SELECT id,name FROM cats";
	while(r.next()) {
		std::string cat_id;
		std::string name;
		r >> cat_id >> name;
		if(in_use_set.find(atoi(cat_id.c_str()))!=in_use_set.end())
			continue;
		cppcms::widgets::checkbox *cb = new cppcms::widgets::checkbox();
		c.form.add_to_cat.attach(cb);
		cb->help(name);
		cb->identification(cat_id);
		c.form.add_to_cat_list.push_back(cb);
	}

	if(is_open)
		c.form.change_status.value(booster::locale::translate("Unpublish"));
	else
		c.form.change_status.value(booster::locale::translate("Publish"));

	if(is_post) {
		c.form.load(context());
		if(c.form.validate()) {
			std::tm now = booster::ptime::local_time(booster::ptime::now());
			if(id==0) {
				if(c.form.remove.value()) {
					response().set_redirect_header(url("/admin/summary"));
					return;
				}
				int open_status = 0;
				if(c.form.change_status.value())
					open_status = 1;
				{
					cppdb::transaction tr(sql());
			
					cppdb::statement st;
					st = sql() << 
						"INSERT INTO posts(author_id,title,abstract,content,publish,is_open) "
						"VALUES(?,?,?,?,?,?)" 
						<< session().get<int>("id")
						<< c.form.title.value()
						<< c.form.abstract.value()
						<< c.form.content.value()
						<< now
						<< open_status << cppdb::exec;
					id = st.sequence_last("post_id_seq");
					for(unsigned i=0;i<c.form.add_to_cat_list.size();i++) {
						cppcms::widgets::checkbox *cb = c.form.add_to_cat_list[i];
						if(!cb->value())
							continue;
						sql()<<	"INSERT INTO post2cat(post_id,cat_id,publish,is_open) "
							"VALUES(?,?,?,?)" 
							<< id
							<< atoi(cb->identification().c_str())
							<< now
							<< open_status
							<< cppdb::exec;
						cache().rise("cat_" + cb->identification());
					}
					if(open_status)
						cache().rise("cat_0"); // insert
					tr.commit();
				}
				if(open_status)
					response().set_redirect_header(url("/blog/post",id));
				else if(c.form.save_and_continue.value())
					response().set_redirect_header(url("/admin/post",id));
				else
					response().set_redirect_header(url("/admin/summary"));
				return;
			} // end of if id == 0
			else {
				if(c.form.remove.value()) {
					cppdb::transaction tr(sql());
					sql() << "DELETE FROM comments WHERE post_id = ?" << id << cppdb::exec;
					sql() << "DELETE FROM post2cat WHERE post_id = ?" << id << cppdb::exec;
					sql() << "DELETE FROM posts where id=?" << id <<cppdb::exec;
					cache().rise("comments");
					std::ostringstream ss;
					ss << "post_" << id;
					cache().rise(ss.str());
					tr.commit();
					response().set_redirect_header(url("/admin/summary"));
					return;
				}
				int open_status = is_open ? 1 : 0;
				if(c.form.change_status.value()) {
					open_status = open_status ^ 1;
				}
				// transaction
				{
					cppdb::transaction tr(sql());
					if(c.form.change_status.value() && open_status) {
						sql()<< "UPDATE posts "
							"SET title=?,abstract=?,content=?,publish=?,is_open=? "
							"WHERE id=?" 
							<< c.form.title.value()
							<< c.form.abstract.value()
							<< c.form.content.value()
							<< now
							<< open_status 
							<< id << cppdb::exec;
						sql()<< "UPDATE post2cat "
							"SET publish=?, is_open=? "
							"WHERE post_id=?" << now << open_status << id << cppdb::exec;
					}
					else {
						sql()<< "UPDATE posts "
							"SET title=?,abstract=?,content=?,is_open=? "
							"WHERE id=?" 
							<< c.form.title.value()
							<< c.form.abstract.value()
							<< c.form.content.value()
							<< open_status 
							<< id << cppdb::exec;

						// if we need this to have correct timestamp synchronized
						sql() << "SELECT publish FROM posts WHERE id=?" << id << cppdb::row >> now;

						if(c.form.change_status.value()) {
							sql()<< "UPDATE post2cat "
								"SET is_open=? "
								"WHERE post_id=?" << open_status << id << cppdb::exec;
						}
					}
					if(c.form.change_status.value()) {
						cache().rise("cat_0"); // publish or unpublish
					}
					
					std::ostringstream ss;
					ss << "post_" << id;
					cache().rise(ss.str());
					
					for(unsigned i=0;i<c.form.add_to_cat_list.size();i++) {
						cppcms::widgets::checkbox *cb = c.form.add_to_cat_list[i];
						if(!cb->value())
							continue;
						sql()<<	"INSERT INTO post2cat(post_id,cat_id,publish,is_open) "
							"VALUES(?,?,?,?)" 
							<< id
							<< atoi(cb->identification().c_str())
							<< now
							<< open_status
							<< cppdb::exec;
						std::ostringstream ss;
						cache().rise("cat_" + cb->identification());
					}
					for(unsigned i=0;i<c.form.del_from_cat_list.size();i++) {
						cppcms::widgets::checkbox *cb = c.form.del_from_cat_list[i];
						if(c.form.change_status.value()) {
							std::ostringstream ss;
							cache().rise("cat_" + cb->identification());
						}
						if(!cb->value())
							continue;
						sql()<<	"DELETE FROM post2cat WHERE post_id=? AND cat_id=?"
							<< id << atoi(cb->identification().c_str())
							<< cppdb::exec;
					}

					tr.commit();
					
				} // end of transaction 

				if(c.form.save.value() || c.form.change_status.value()) {
					if(open_status)
						response().set_redirect_header(url("/blog/post",id));
					else
						response().set_redirect_header(url("/admin/summary"));
				}
				else {
					response().set_redirect_header(url("/admin/post",id));
				}
			}  // else if id
		} // if valid
	} // if post
	master::prepare(c);
	render("admin_skin","post",c);
}



} // admin
} // apps
