#include "options.h"
#include "wiki.h"

namespace apps {

using namespace dbixx;

void global_options::load(archive &a) {
	a>>users_only_edit;
}
void global_options::save(archive &a) const
{
	a<<users_only_edit;
}
void locale_options::load(archive &a)
{
	a>>title>>about>>copyright;
}
void locale_options::save(archive &a) const
{
	a<<title<<about<<copyright;
}


options::options(wiki &w):
	master(w)
{
	wi.url_next.add("^/options/?$",
		boost::bind(&options::edit,this));
	wi.on_load(boost::bind(&options::reset,this));
	reset();
}

void options::reset()
{
	loaded=false;
}

string options::edit_url()
{
	return wi.root()+"/options/";
}

void options::load()
{
	if(loaded)
		return;
	if(!wi.cache.fetch_data("global_ops",global)) {
		wi.sql<<"SELECT value FROM options "
			"WHERE	lang='global' AND name='users_only_edit' ";
		row r;
		if(wi.sql.single(r)) {
			string v;
			r >> v;
			global.users_only_edit=atoi(v.c_str());
		}
		else { 
			global.users_only_edit=0;
		}
		wi.cache.store_data("global_ops",global);
	}
	if(wi.cache.fetch_data("local_ops:"+locale,local))
		return;
	result res;
	wi.sql<<"SELECT value,name FROM options "
		"WHERE  lang=?",locale,res;
	row r;
	while(res.next(r)) {
		string v,n;
		r>>v>>n;
		if(n=="title")
			local.title=v;
		else if(n=="about")
			local.about=v;
		else if(n=="copyright")
			local.copyright=v;
	}
	if(local.title.empty())
		local.title=wi.gettext("Wiki++ &mdash; CppCMS Wiki");
	if(local.about.empty())
		local.about=
			wi.gettext("## About\n"
				"\n"
				"Wiki++ is a wiki engine powered by\n"
				"[CppCMS](http://cppcms.sf.net/) web development framework.\n");
	if(local.copyright.empty())
		local.copyright=wi.gettext("&copy; All Rights Reserverd");
	wi.cache.store_data("local_ops:"+locale,local);
}


void options::save()
{
	wi.sql<<"DELETE FROM options "
		"WHERE lang='global' OR lang=?",
		locale,exec();
	wi.sql<<"INSERT INTO options(value,name,lang) "
		"VALUES(?,'users_only_edit','global')",
		global.users_only_edit,exec();
	wi.sql<<"INSERT INTO options(value,name,lang) "
		"VALUES(?,'title',?)",
		local.title,locale,exec();
	wi.sql<<"INSERT INTO options(value,name,lang) "
		"VALUES(?,'about',?)",
		local.about,locale,exec();
	wi.sql<<"INSERT INTO options(value,name,lang) "
		"VALUES(?,'copyright',?)",
		local.copyright,locale,exec();
	wi.cache.rise("global_ops");
	wi.cache.rise("local_ops:"+locale);
}

void options::edit()
{
	if(!wi.users.auth()) {
		wi.users.error_forbidden();
		return;
	}
	data::edit_options c(&wi);
	if(wi.env->getRequestMethod()=="POST") {
		c.form.load(*wi.cgi);
		if(c.form.validate()) {
			global.users_only_edit=c.form.users_only.get();
			local.title=c.form.wiki_title.get();
			local.copyright=c.form.copyright.get();
			local.about=c.form.about.get();
			save();
		}
	}
	else {
		load();
		c.form.users_only.set(global.users_only_edit);
		c.form.wiki_title.set(local.title);
		c.form.copyright.set(local.copyright);
		c.form.about.set(local.about);
	}
	ini(c);
	wi.render("edit_options",c);
}


}

