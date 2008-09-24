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
#include "base_view.h"
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

	string current_template;

	inline void use_template(string s="") { current_template=s; };

	template<typename T>
	auto_ptr<T> view()
	{
		using cppcms::details::views_storage;
		auto_ptr<T> ptr;
		if(current_template!="") {
			base_view_impl *p=views_storage::instance().fetch_view(current_template,typeid(T).name());
			T *T_ptr=NULL;
			if(p) {
				T_ptr=dynamic_cast<T*>(p);
				if(!T_ptr) delete p;
			}
			if(!T_ptr) 
				throw cppcms_error("Can't find view "+string(typeid(T).name())+" in "+current_template);
			ptr.reset(T_ptr);
		}
		else 
			ptr.reset(new T);
		ptr->set_worker(this);
		return ptr;
	};

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
