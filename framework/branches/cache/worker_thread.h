#ifndef _WORKER_THREAD_H_
#define _WORKER_THREAD_H_

#include <pthread.h>
#include <sstream>
#include <string>

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTTPStatusHeader.h"
#include "cgicc/HTMLClasses.h"
#include <memory>

#include "FCgiIO.h"
#include "cppcms_error.h"
#include "url.h"


namespace cppcms {

using namespace std;
using cgicc::CgiEnvironment;
using cgicc::FCgiIO;
using cgicc::Cgicc;
using cgicc::HTTPHeader;


class worker_thread {
friend class url_parser;
protected:	
	auto_ptr<FCgiIO>io;
	auto_ptr<Cgicc> cgi;
	CgiEnvironment const *env;

	string out;
	auto_ptr<HTTPHeader> response_header;
	void set_header(HTTPHeader*h){response_header=auto_ptr<HTTPHeader>(h);};
	virtual void main();
public:
	int id;
	pthread_t pid;

	void run(FCGX_Request *req);

	worker_thread();
	virtual ~worker_thread(){ };
	virtual void init() { };
};

}

#endif
