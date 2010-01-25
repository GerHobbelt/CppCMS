#include "wiki.h"
#include "master.h"
#include "master_content.h"
#include "cxxmarkdown/markdowncxx.h"
#include <cppcms/localization.h>
#include <cppcms/service.h>

#define _(X) ::cppcms::locale::translate(X)
#define N_(S,P,N)  ::cppcms::locale::translate(S,P,N)

namespace content {

string master::markdown(string s)
{
	string tmp;
	markdown2html(s,tmp);
	return tmp;
}

}

namespace apps {

master::master(wiki &_w) : 
	application(_w.service()),
	wi(_w),
	sql(wi.sql),
	locale_name(wi.locale_name)
{
	json::object langs=settings().get("wikipp.languages",json::object());
	for(json::object::const_iterator p=langs.begin(),e=langs.end();p!=e;++p) {
		string lname;
		if(p->first=="en")
			lname="English";
		else {
			/// Translate as the target language
			/// for fr gettext("LANG")="Francis"
			lname=_("LANG").str<char>(service().generator().get(p->second.str()));
			if(lname=="LANG") {
				lname=p->first;
			}
		}
		languages[lname]=wi.page.default_page_url(p->first);
	}
}

void master::ini(content::master &c)
{
	wi.options.load();
	c.media=settings().get<std::string>("wikipp.media");
	c.syntax_highlighter=settings().get("wikipp.syntax_highlighter","");
	c.cookie_prefix=settings().get("session.cookies_prefix","cppcms_session")+"_";
	c.main_link=wi.page.default_page_url();
	c.main_local=wi.page.default_page_url(locale_name);
	c.toc=wi.index.index_url();
	c.changes=wi.index.changes_url();
	c.login_link=wi.users.login_url();
	c.wiki_title=wi.options.local.title;
	c.about=wi.options.local.about;
	c.copyright=wi.options.local.copyright;
	c.contact=wi.options.global.contact;
	c.edit_options=wi.options.edit_url();
	c.languages=languages;
}


}
