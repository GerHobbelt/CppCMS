#include "wiki.h"
#include "master.h"
#include "master_data.h"
#include "cxxmarkdown/markdowncxx.h"

namespace data {

string master::markdown(string s)
{
	string tmp;
	markdown2html(s,tmp);
	return tmp;
}

}

namespace apps {

master::master(wiki &_w) : 
	application(_w.worker),
	wi(_w),
	sql(wi.sql),
	locale(wi.locale)
{
}

void master::ini(data::master &c)
{
	wi.options.load();
	c.media=app.config.sval("wikipp.media");
	c.cookie_prefix=app.config.sval("session.cookies_prefix","cppcms_session")+"_";
	c.main_link=wi.page.default_page_url();
	c.main_local=wi.page.default_page_url(locale);
	c.toc=wi.index.index_url();
	c.changes=wi.index.changes_url();
	c.login_link=wi.users.login_url();
	c.wiki_title=wi.options.local.title;
	c.about=wi.options.local.about;
	c.copyright=wi.options.local.copyright;
	c.contact=wi.options.global.contact;
	c.edit_options=wi.options.edit_url();
	vector<string> const &langs=app.config.slist("locale.lang_list");
	for(vector<string>::const_iterator p=langs.begin(),e=langs.end();p!=e;++p) {
		string lname;
		if(*p=="en")
			lname="English";
		else {
			/// Translate as the target language
			/// for fr gettext("LANG")="Francis"
			set_lang(*p);
			lname=gettext("LANG");
			if(lname=="LANG") {
				lname=*p;
			}
		}
		c.languages[lname]=wi.page.default_page_url(*p);
	}
	set_lang(locale);
}


}
