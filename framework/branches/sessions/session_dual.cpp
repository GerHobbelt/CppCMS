#include "session_dual.h"

namespace cppcms {

session_dual::save(session_interface *session,string const &data,time_t timeout)
{
	if(data > limit) {
		server->save(session,data,timeout);
	}
	else {
		if(session->get_session_cookie().size() == 32) {
			server->clear(session);
		}
		client->save(session,data,timeout);
	}
}

session_dual::load(session_interface *session,string &data,time_t &timeout)
{
	if(session->get_session_cookie().size()==32) {
		server->load(session,data,timeout);
	}
	else {
		client->load(session,data,timeout);
	}
}

session_dual::clear(session_interface *session)
{
	if(session->get_session_cookie().size()==32) {
		server->clear(session);
	}
	else {
		client->clear(session);
	}
}

} // cppcms
