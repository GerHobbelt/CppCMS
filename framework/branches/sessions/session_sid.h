#ifndef CPPCMS_SESSION_SID_H
#define CPPCMS_SESSION_SID_H

namespace cppcms {

namespace details {
class sid_generator : public boost::noncopyable {
	uint64_t session_counter;
	char uid[16];
public:
	sid_generator();
	std::string operator();
};
}

class session_sid : public session_api {
	details::sid_generator sid;
	boost::shared_ptr<session_server_storage> storage;
public:
	session_sid(boost::shared_ptr<session_server_storage> s) :
		storage(s)
	{
	}
	session_sid();
	void save(session_interface *,string const &data,time_t timeout);
	bool load(session_interface *,string &data);
	void clear(session_inteface *);

};
}


#endif
