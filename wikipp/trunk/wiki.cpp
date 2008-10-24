#include "wiki.h"

wiki::wiki(manager const &s) :  worker_thread(s)
{
	sql.driver("mysql");
	sql.param("dbname",app.config.sval("mysql.db"));
	sql.param("username",app.config.sval("mysql.user"));
	sql.param("password",app.config.sval("mysql.pass"));
	sql.connect();
	
	string root=app.config.sval("wikipp.script");

	url.add("^/(\\w+)(/.*)$",
		boost::bind(&wiki::lang,this,$1,$2));
	url2.add("/(\\w+)/?",
		boost::bind(wiki::page,this,$1));
	links.page=root+"/%1%/%2%";
	links.main_page=root+"/en/main";
	url2.add("/(\\w+)/edit/?",
		boost::bind(wiki::edit_page,this,$1));
	links.edit_page=page+"/edit/";

	use_template("view");
}

wiki::lang(string lang,string url)
{
	if(lang!="en") {
		vector<string> const &lst=app.config.slist("locale.lang_list");
		vector<string>::const_iterator p,e;
		for(p=lst.begin(),e=lst.end();p!=e;++p) {
			if(lang==*p) {
				break;
			}
		}
		if(p==e) {
			set_header(new HTTPRedirectHeader(links.main_page));
			return;
		}
	}
	locale=lang;
	set_lang(lang);
	if(url2.parse(url)<0) {
		set_header(new HTTPRedirectHeader(links.main_page));
	}
}

void wiki::main()
{
	if(url.parse()<0) {
		set_header(new HTTPRedirectHeader(links.main_page));
	}
}

void wiki::page(string slug)
{
	string key="article_"+locale+":"+slug;
	if(cache.fetch_page(key))
		return;
	data::page c;

	sql<<"SELECT title,content WHERE lang=? AND slug=?" ,locale,slug;
	row r;
	if(!sql.single(r)) {
		string redirect=(boost::format(links.edit_page) % lang % slug ).str();
		set_header(new HTTPRedirectHeader(redirect));
		return;
	}
	ini_share(c);
	r >> c.title >> c.content;
	render("page",c);
	cache.store(key);
}

void wiki::edit_page(string slug)
{
	data::edit_page c(this);
	if(env->getRequestMethod()=="POST") {
		c.form.load();
		if(c.form.validate()) {
			if(c.form.save.pressed || c.form.save_cont.pressed) {
				transaction tr(sql);
				sql<<"SELECT id,users_only FROM pages where lang=? and slug=?",locale,slug;
				row r;
				int id,users_only;
				if(sql.single(r)) {
					r>>id>>users_only;
					// TODO Users only
					time_t now;
					time(&now);
					std::tm t;
					localtime_r(&now,&t);
					sql<<	"INSERT into history(id,created,version,title,content) "
						"vlaues(?,?,"
							"(select count(*) from history where id=?),"
							"(select title from pages where id=?),"
							"(select content from pages where id=?))",
								id,t,id,id,id,exec();
					sql<<	"UDPATE pages set content=?,title=? where lang=? and slug=?",
						c.form.content.get(),c.form.title.get(),locale,slug,exec();
					tr.commit();
							
				}
				else {
					sql<<	"INSERT into pages(lang,slug,title,content,0) "
						"values(?,?,?,?)",
						locale,slug,c.form.title.get(),c.form.content.get(),exec();
				}

			}
			if(c.form.save.pressed) {
				string red=str(boost::format(links.page) % locale % slug);
				set_header(new HTTPRedirectHeader(red));
				return;
			}
		}
	}
	else {
		sql<<"SELECT title,content from pages where lang=? and slug=?",locale,slug;
		row r;
		if(sql.single(r)) {
			r>>c.form.title.str()>>c.form.content.str();
		}
		else {
			c.new_page=true;
		}
	}
	ini_edit_page(c);
	render("edit_page",c);
}

void wiki::main()
{
	data::master c;
	render("master",c);
}


