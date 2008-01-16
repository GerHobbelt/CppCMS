#ifndef _WORKER_THREAD_H_
#define _WORKER_THREAD_H_

#include <pthread.h>
#include <sstream>
#include <string>

#include "textstream.h"

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTTPStatusHeader.h"
#include "cgicc/HTMLClasses.h"

#include "cache.h"

#include <memory>

#include "FCgiIO.h"
#include "http_error.h"
#include "url.h"

using namespace std;
using cgicc::CgiEnvironment;
using cgicc::FCgiIO;
using cgicc::Cgicc;
using cgicc::HTTPHeader;


class Worker_Thread {
friend class URL_Parser;
protected:	
	auto_ptr<FCgiIO>io;
	auto_ptr<Cgicc> cgi;
	CgiEnvironment const *env;

	Text_Stream out;
	auto_ptr<HTTPHeader> response_header;
	void set_header(HTTPHeader*h){response_header=auto_ptr<HTTPHeader>(h);};
	virtual void main();
	
	bool out_gzip;
	bool is_cached;
	
	string string_cache;
	string gzip_cache;
	
	bool check_gzip(FCGX_Request *fcgi);
	void flush_output(FCgiIO &io);
	
	bool cache_try(string const &key);
	void cache_store(string const &key,time_t timeout=365*24*3600);
	void cache_drop(string const &key);

public:
	int id;
	pthread_t pid;


	void run(FCGX_Request *req);

	Worker_Thread();
	virtual ~Worker_Thread(){ };
	virtual void init() { };
};

#endif
