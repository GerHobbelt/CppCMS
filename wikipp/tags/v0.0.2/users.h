#ifndef USERS_H
#define USERS_H

#include "master.h"

namespace apps {

using namespace std;

class users : public master {
	bool auth_done;
	bool auth_ok;
	bool disable_reg;

	void login();
	void do_auth();
	void new_user();
	void reset();
public:
	bool user_exists(string);
	bool check_login(string,string);
	bool auth();
	void error_forbidden();
	string username;
	string login_url();
	users(wiki &);
	
};


}
#endif


