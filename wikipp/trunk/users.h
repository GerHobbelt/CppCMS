#ifndef USERS_H
#define USERS_H

#include "data.h"
#include "master.h"

namespace apps {

using namespace std;

class users : public master {
	bool auth_done;
	bool auth_ok;

	void login();
	void do_auth();
	void reset();
public:
	bool check_login(string,string);
	bool auth();
	void error_forbidden();
	string login_url();
	users(wiki &);
	
};


}
#endif


