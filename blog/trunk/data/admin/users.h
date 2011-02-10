#pragma once

#include <data/admin/master.h>

namespace data {
	namespace admin {
		struct users : public master {
			struct login_form : public cppcms::form {
				cppcms::widgets::text username;
				cppcms::widgets::password password;
				cppcms::widgets::submit login;

				login_form()
				{
					username.non_empty();
					username.message(booster::locale::translate("Username"));
					password.non_empty();
					password.message(booster::locale::translate("Password"));
					login.value(booster::locale::translate("Login"));

					add(username);
					add(password);
					add(login);
				}
			};

			login_form login;
		};
	}
}
