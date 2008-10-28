#include "wiki.h"
#include "master.h"
#include "data.h"

master::master(wiki &_w) : wi(_w)
{
}

void master::ini(data::master &c)
{
	c.media=wi.app.config.sval("wikipp.media");
	c.cookie_prefix=wi.app.config.sval("wikipp.cookie_id","");
	c.main_link=wi.page_app.default_page_url();
	c.toc=wi.links.admin_url(wi.links.toc).str();
	c.login_link=wi.links.admin_url(wi.links.login).str();
	c.wiki_title=wi.ops.local.title;
	c.about=wi.ops.local.about;
	c.copyright=wi.ops.local.copyright;
	c.edit_options=wi.links.admin_url(wi.links.edit_options).str();
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
		c.languages[lname]=wi.page_app.default_page_url(*p);
	}
	wi.set_lang(wi.locale);
}

