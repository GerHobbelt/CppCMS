#ifndef OPTIONS_CONTENT_H
#define OPTIONS_CONTENT_H

#include "master_content.h"

namespace content {

struct options_form : form {
	widgets::checkbox users_only;
	widgets::text contact_mail;
	widgets::text wiki_title;
	widgets::textarea about;
	widgets::text copyright;
	widgets::submit submit;
	options_form();
};


struct edit_options:  public master {
	options_form form;
};


} // namespace content

#endif

