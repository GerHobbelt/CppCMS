#include "session_sid.h"
#include "md5.h"

namespace cppcms {
namespace details {

sid_generator::sid_generator()
{
	session_counter=0;
	ifstream urandom("/dev/urandom");
	if(!urandom.good() || urandom.get(uid,16).fail()) {
		throw cppcms_error("Failed to read /dev/urandom");
	}
}

std::string sid_generator::operator()
{
	md5_state_t st;
	char res[33];
	unsigned char md5[16];
	md5_init(&st);
	md5_append(&st,16);
	md5_append(&session_counter,8);
	session_counter++;
	md5_finish(&sti,md5);
	for(i=0;i<16;i++) {
		snprintf(res+i*2,3,"%02x",md5[i]);
	}
	return std::string(res);
}


} // namespace details


session_sid::session_sid(worker_thread &w) :
	session_api(w)
{
	string backend=w.app.config("session.storage_backend","");
	if(backend=="files") {
				
	}
}

}
