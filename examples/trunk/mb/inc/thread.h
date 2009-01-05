#ifndef THREAD_H
#define THREAD_H
#include <cppcms/application.h>
#include <string>

namespace data { class base_thread; }

namespace apps {
class mb;
using std::string;

class thread : public cppcms::application {
	mb &board;
	void flat(string id);
	void tree(string id);
	void reply(string);
	string reply_url(int msg_id);
	int ini(string sid,data::base_thread &);
public:
	thread(mb &);
	string flat_url(int id);
	string tree_url(int id);
	string user_url(int id);
};

} // apps


#endif
