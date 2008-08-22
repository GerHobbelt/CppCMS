#ifndef _WORKER_THREAD_H_
#define _WORKER_THREAD_H_

#include <pthread.h>
#include <sstream>
#include <string>

#include <cgicc/Cgicc.h>
#include <cgicc/HTTPHTMLHeader.h>
#include <cgicc/HTTPStatusHeader.h>
#include <cgicc/HTMLClasses.h>
#include <boost/noncopyable.hpp>
#include <memory>

#include "cppcms_error.h"
#include "url.h"
#include "cache_interface.h"
#include "base_cache.h"
#include "cgicc_connection.h"
#include "transtext.h"

namespace cppcms {

class manager;

using namespace std;
using cgicc::CgiEnvironment;
using cgicc::Cgicc;
using cgicc::HTTPHeader;

class worker_thread: private boost::noncopyable {
	friend class url_parser;
	friend class cache_iface;
	friend class base_view_impl;

	auto_ptr<HTTPHeader> response_header;
	list<string> other_headers;
	base_cache *caching_module;
	bool gzip;
	bool gzip_done;
	stringbuf out_buf;

	transtext::trans const *gt;

protected:

	url_parser url;
	manager const &app;
	Cgicc *cgi;
	CgiEnvironment const *env;

	cache_iface cache;
	ostream cout;

	void set_header(HTTPHeader *h);
	void add_header(string s);
	void set_lang();
	void set_lang(string const &s);

	inline char const *gettext(char const *s) { return gt->gettext(s); };
	inline char const *ngettext(char const *s,char const *p,int n) { return gt->ngettext(s,p,n); };

	virtual void main();
public:
	int id;
	pthread_t pid;

	void run(cgicc_connection &);

	worker_thread(manager const &s);
	virtual ~worker_thread();
};

}

#endif
