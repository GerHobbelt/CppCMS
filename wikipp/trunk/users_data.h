#ifndef USERS_DATA_H
#define USERS_DATA_H

#include "master_data.h"

namespace data {

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
	widgets::submit submit;
	list<widgets::checkbox> quiz;
	new_user_form(wiki *w);
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

} // namespace data

#endif
