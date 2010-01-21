#ifndef USERS_CONTENT_H
#define USERS_CONTENT_H

#include "master_content.h"

namespace content {

struct login_form : public form {
	wiki *w;
	widgets::text username;
	widgets::password password;
	widgets::submit login;
	login_form(wiki *);
	virtual bool validate();
};

struct new_user_form : public form {
	wiki *w;
	widgets::text username;
	widgets::password password1;
	widgets::password password2;
	widgets::text captcha;
	widgets::submit submit;
	new_user_form(wiki *w);
	void generate_captcha();
	bool virtual validate(); 
};

struct new_user : public master {
	new_user_form form;
	new_user(wiki *w) : form(w){};
};

struct login : public master {
	login_form form;
	string new_user;
	login(wiki *w) : form(w){};
};

} // namespace content

#endif
