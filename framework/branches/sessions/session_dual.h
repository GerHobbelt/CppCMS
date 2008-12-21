#ifndef CPPCMS_SESSION_DUAL_H
#define CPPCMS_SESSION_DUAL_H

#include "session_api.h"

namespace cppcms {

class session_dual : public session_api {
	boost::shared_ptr<session_api> 	client;
	boost::shared_ptr<session_api>  server;
	size_t limit;
public:
	session_api(boost::shared_ptr<session_api> c,boost::shared_ptr<session_api> s,size_t l) :
		cookies(c),
		server(s),
		limit(l)
	{
	}
	virtual void save(session_interface *,std::string const &data,time_t timeout)
	virtual bool load(session_interface *,std::string &data,time_t &timeout);
	virtual void clear(session_interface *);

}


#endif
