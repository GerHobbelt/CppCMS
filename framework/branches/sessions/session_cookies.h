#ifndef CPPCMS_SESSION_COOKIES_H
#define CPPCMS_SESSION_COOKIES_H
#include "session_api.h"
#include <memory>
#include <string>

namespace cppcms {
class encryptor;
class worker_thread;
class ession_interface;

class session_cookies : public session_api {
	std::auto_ptr<encryptor> encr;
public:
	session_cookies(worker_thread &w);
	virtual void save(session_interface *,std::string const &data,time_t timeout);
	virtual bool load(session_interface *,std::string &data);
	virtual void clear(session_inteface *);
	~session_api();
};


} // cppcms

#endif
