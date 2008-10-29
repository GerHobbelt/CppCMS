#include "wiki.h"
#include "master.h"
#include "data.h"

namespace apps {

master::master(wiki &_w) : 
	wi(_w),
	locale(wi.locale)
{
}

void master::ini(data::master &c)
{
	wi.options.load();
	c.media=wi.app.config.sval("wikipp.media");
	c.cookie_prefix=wi.app.config.sval("wikipp.cookie_id","");
	c.main_link=wi.page.default_page_url();
	c.toc=wi.index.index_url();
	c.login_link=wi.users.login_url();
	c.wiki_title=wi.options.local.title;
	c.about=wi.options.local.about;
	c.copyright=wi.options.local.copyright;
	c.edit_options=wi.options.edit_url();
	vector<string> const &langs=wi.app.config.slist("locale.lang_list");
	for(vector<string>::const_iterator p=langs.begin(),e=langs.end();p!=e;++p) {
		string lname;
		if(*p=="en")
			lname="English";
		else {
			/// Translate as the target language
			/// for fr gettext("LANG")="Francis"
			wi.set_lang(*p);
			lname=wi.gettext("LANG");
			if(lname=="LANG") {
				lname=*p;
			}
		}
		c.languages[lname]=wi.page.default_page_url(*p);
	}
	wi.set_lang(locale);
}


}
