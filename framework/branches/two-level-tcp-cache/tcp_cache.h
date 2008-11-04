#ifndef TCP_CHACHE_H
#define TCP_CHACHE_H
#include "base_cache.h"
#include "cache_interface.h"
#include <string>

namespace cppcms {

using namespace std;

class messenger;
struct tcp_operation_header;

class tcp_cache : public base_cache {
	messenger *tcp;
	base_cache *L1;
	cache_factory *L1_factory;
	int conns;
	messenger &get(string const &key);
	void broadcast(tcp_operation_header &h,string &data);
public:
	tcp_cache(vector<string> const &ip_list,vector<long> const &port_list,cache_factory *f=NULL);

	virtual bool fetch_page(string const &key,string &output,bool gzip,time_t &);
	virtual bool fetch_page(string const &key,string &output,bool gzip);
	virtual bool fetch(string const &key,archive &a,set<string> &tags,time_t &);
	virtual int rise(string const &trigger,list<string> *);
	virtual void remove(string const &s);
	virtual void clear();
	virtual void stats(unsigned &keys,unsigned &triggers);
	virtual void store(string const &key,set<string> const &triggers,time_t timeout,archive const &a);
	virtual ~tcp_cache();
};

class l1_cleanup_service;

class tcp_cache_factory : public cache_factory {
	vector<string> ip;
	vector<long>   port;
	auto_ptr<cache_factory> L1_factory;
	auto_ptr<l1_cleanup_service> cleanup;
public:
	tcp_cache_factory(vector<string> const &_ip,vector<long> const &_port,auto_ptr<cache_factory> l1);
	virtual base_cache *get() const;
	virtual void del(base_cache *p) const;

	~tcp_cache_factory();
};

}

#endif
