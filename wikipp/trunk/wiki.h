#ifndef WIKI_H
#define WIKI_H

#include "data.h"
#include <cppcms/worker_thread.h>

using namespace cppcms;

class wiki : public worker_thread {
public:
	wiki(manager const &s) :  worker_thread(s)
	{
		use_template("view");
	};
	virtual void main();
};


#endif

