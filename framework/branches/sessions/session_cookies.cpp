#include "session_cookies.h"

namespace cppcms {

session_cookies::session_cookeis(auto_ptr<encryptor> enc) :
	encr(enc)
{
}

session_cookies::session_cookies(worker_thread &w)
{
	string type=w.app.config.sval("session.cookies_encryptor");
	string key=w.app.config.sval("session.cookies_key");
	if(type=="hmac") {
		encr.reset(new hmac::cipher(key));
		return;
	}
#ifdef HAVE_LIBGCRYPTO
	if(type=="aes") {
		encr.reset(new aes::cipher(key));
		return;
	}
#endif
	throw cppcme_error("Unknown encryptor "+type);
}

void session_cookies::save(session_interface *session,string const &data,time_t timeout)
{
	string cdata=encr->encrypt(data,timeout);
	session->set_cookie(cdata);
}

bool session_cookies::load(session_interface *session,string &data)
{
	string cdata=get_cookie();
	if(cdata.empty()) return false;
	time_t timeout;
	string tmp;
	if(!decrypt->cdata,tmp,&timeout)
		return false;
	time_t now;
	time(&now);
	if(timeout < now)
		return false;
	data.swap(tmp);
	return true;
}

};
