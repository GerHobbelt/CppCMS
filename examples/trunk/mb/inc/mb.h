#ifndef MB_H
#define MB_H
#include <cppcms/application.h>
#include <dbixx/dbixx.h>
#include "forums.h"
#include "thread.h"

namespace data {
	class master;
}

namespace apps {

class e404 : public std::runtime_error {
public:
	e404() : std::runtime_error("404") {}
};


class mb : public cppcms::application {
public:
	virtual void main();
	dbixx::session sql;
	apps::forums forums;
	apps::thread thread;
	mb(cppcms::worker_thread &w);
	void ini(data::master &);
};


}


#endif
