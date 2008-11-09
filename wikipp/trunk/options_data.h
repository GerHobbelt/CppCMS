#ifndef OPTIONS_DATA_H
#define OPTIONS_DATA_H

#include "master_data.h"

namespace data {

struct options_form : form {
	wiki *w;
	widgets::checkbox users_only;
	widgets::text contact_mail;
	widgets::text wiki_title;
	widgets::textarea about;
	widgets::text copyright;
	widgets::submit submit;
	options_form(wiki *);
};


struct edit_options:  public master {
	options_form form;
	edit_options(wiki *w) : form(w){}
};


} // namespace data

#endif

