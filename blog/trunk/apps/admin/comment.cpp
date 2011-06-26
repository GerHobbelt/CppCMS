#include <apps/admin/comment.h>
#include <data/admin/comment.h>
#include <cppdb/frontend.h>

#include <cppcms/url_mapper.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>

#include <stdlib.h>

namespace apps {
namespace admin {


comment::comment(cppcms::service &s) : master(s)
{
	mapper().assign("/{1}");
	dispatcher().assign("/(\\d+)/?",&comment::prepare,this,1);
}

void comment::prepare(std::string sid)
{
	int id = atoi(sid.c_str());
	data::admin::comment c;
	c.id = id;

	if(request().request_method()=="POST") {
		c.form.load(context());
		if(c.form.validate()) {
			cppdb::transaction tr(sql());
			
			cppdb::result r;

			r = sql()<<
				"SELECT post_id FROM comments "
				"WHERE id = ?" << id << cppdb::row;
			if(r.empty()) {
				response().make_error_response(404);
				return;
			}

			int post_id = r.get<int>(0);
			c.post_id = post_id;
			
			r.clear();

			if(c.form.erase.value()) {
				sql()<<	"DELETE FROM comments "
					"WHERE id=?" << id << cppdb::exec;
				sql()<< "UPDATE posts "
					"SET comment_count = "
					"	(SELECT count(*) FROM comments WHERE post_id=?) "
					"WHERE id = ? " << post_id <<post_id<< cppdb::exec;
			}
			else {
				sql()<<	"UPDATE comments "
					"SET author = ?,email = ?,url = ?,content = ? "
					"WHERE id = ?" 
					<<c.form.author.value() << c.form.mail.value() 
					<< c.form.url.value() << c.form.content.value()
					<< id << cppdb::exec;
			}
			std::ostringstream ss;
			ss << "post_" << post_id;
			cache().rise(ss.str());
			cache().rise("comments");
			
			tr.commit();

			response().set_redirect_header(url("/blog/post",post_id));
			return;

		}
	}
	else {
		cppdb::result r;
		
		r = sql()<<
			"SELECT post_id,author,email,url,content FROM comments "
			"WHERE id = ?" << id << cppdb::row;
		if(r.empty()) {
			response().make_error_response(404);
			return;
		}
		c.post_id = r.get<int>(0);
		c.form.author.value(r.get<std::string>(1));
		c.form.mail.value(r.get<std::string>(2));
		c.form.url.value(r.get<std::string>(3));
		c.form.content.value(r.get<std::string>(4));
	}

	master::prepare(c);
	render("admin_skin","comment",c);
}


} // admin
} // apps
