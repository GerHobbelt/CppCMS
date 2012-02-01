#include "options.h"
#include "wiki.h"
#include "options_data.h"

namespace data {
options_form::options_form(wiki *_w):
	w(_w),
	users_only("uonly",w->gettext("Users Only")),
	contact_mail("contact",w->gettext("Contact e-mail")),
	wiki_title("wtitle",w->gettext("Wiki Title")),
	about("about",w->gettext("About Wiki")),
	copyright("copy",w->gettext("Copyright String")),
	submit("submit",w->gettext("Submit"))
{
	*this & users_only & contact_mail & wiki_title & copyright & about & submit;
	wiki_title.set_nonempty();
	copyright.set_nonempty();
	contact_mail.set_nonempty();
	about.set_nonempty();
	about.rows=10;
	about.cols=40;
	users_only.help=w->gettext("Disable creation of new articles by visitors");
}

} // namespace data

namespace apps {

using namespace dbixx;

void global_options::load(archive &a) {
	a>>users_only_edit>>contact;
}
void global_options::save(archive &a) const
{
	a<<users_only_edit<<contact;
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
	on_start.connect(boost::bind(&options::reset,this));
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
	global.users_only_edit=0;
	global.contact.clear();
	local.about.clear();
	local.title.clear();
	local.copyright.clear();
	if(!cache.fetch_data("global_ops",global)) {
		result res;
		sql<<	"SELECT name,value FROM options "
			"WHERE	lang='global' ",res;
		row r;
		while(res.next(r)) {
			string n,v;
			r >> n >> v;
			if(n=="users_only_edit")
				global.users_only_edit=atoi(v.c_str());
			else if(n=="contact")
				global.contact=v;
		}
		if(global.contact.empty())
			global.contact="no@mail";
		cache.store_data("global_ops",global);
	}
	if(cache.fetch_data("local_ops:"+locale,local))
		return;
	result res;
	sql<<	"SELECT value,name FROM options "
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
		local.title=gettext("Wiki++ &mdash; CppCMS Wiki");
	if(local.about.empty())
		local.about=
			gettext("## About\n"
				"\n"
				"Wiki++ is a wiki engine powered by\n"
				"[CppCMS](http://cppcms.sf.net/) web development framework.\n");
	if(local.copyright.empty())
		local.copyright=gettext("&copy; All Rights Reserverd");
	cache.store_data("local_ops:"+locale,local);
	loaded=true;
}


void options::save()
{
	sql<<	"DELETE FROM options "
		"WHERE lang='global' OR lang=?",
		locale,exec();
	sql<<	"INSERT INTO options(value,name,lang) "
		"VALUES(?,'users_only_edit','global')",
		global.users_only_edit,exec();
	sql<<	"INSERT INTO options(value,name,lang) "
		"VALUES(?,'contact','global')",
		global.contact,exec();
	sql<<	"INSERT INTO options(value,name,lang) "
		"VALUES(?,'title',?)",
		local.title,locale,exec();
	sql<<	"INSERT INTO options(value,name,lang) "
		"VALUES(?,'about',?)",
		local.about,locale,exec();
	sql<<	"INSERT INTO options(value,name,lang) "
		"VALUES(?,'copyright',?)",
		local.copyright,locale,exec();
	cache.rise("global_ops");
	cache.rise("local_ops:"+locale);
}

void options::edit()
{
	if(!wi.users.auth()) {
		wi.users.error_forbidden();
		return;
	}
	data::edit_options c(&wi);
	if(env->getRequestMethod()=="POST") {
		c.form.load(*cgi);
		if(c.form.validate()) {
			global.users_only_edit=c.form.users_only.get();
			global.contact=c.form.contact_mail.get();
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
		c.form.contact_mail.set(global.contact);
	}
	ini(c);
	render("edit_options",c);
}


}

