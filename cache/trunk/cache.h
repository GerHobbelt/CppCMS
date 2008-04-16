#ifndef CACHE_H
#define CACHE_H

#include <pthread.h>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <string>
#include <list>
#include <vector>
#include <map>

namespace cache {

using boost::shared_ptr;
using boost::shared_array;
using namespace std;

class base_cache {
protected:
	string deflate(string const &s);
	void split_to_keys(string const &s,vector<string> &v);
public:
	virtual string insert(string const &key,string const &sec,
				string const &input,time_t timeout=365*24*3600,bool no_gzip=false)
	{
		return deflate(input);
	};
	virtual bool fetch_string(string const &key,string &output) { return false; };
	virtual bool fetch_gzip(string const &key,string &output) { return false; };
	virtual void drop_primary(string const &key) {};
	virtual void drop_secondary(string const &key) {};
	virtual ~base_cache(){};
};

class thread_cache : public base_cache
{
	pthread_mutex_t lru_lock;
	pthread_rwlock_t lock;
	unsigned limit;

	struct container {
		typedef map<string,container>::iterator primary_ptr;
		typedef multimap<time_t,primary_ptr>::iterator timeout_ptr;
		typedef list<primary_ptr>::iterator lru_ptr;

		string original,gzipped;
		bool has_gzipped;
		timeout_ptr time_entry;
		lru_ptr lru_entry;
	};

	map<string,container> primary;
	typedef map<string,container>::iterator primary_ptr;
	multimap<time_t,primary_ptr> timeout;
	list<primary_ptr> lru;

	multimap<string,primary_ptr> sec2prim;
	typedef multimap<string,primary_ptr>::iterator sec2prim_ptr;
	multimap<string,sec2prim_ptr> prim2sec;
	typedef multimap<string,sec2prim_ptr>::iterator prim2sec_ptr;

	void insert(container &c, string const &primary,vector<string> const &secondary,time_t lifetime);

	void remove_prim(primary_ptr p);
	void remove_sec_by_prim(string const &k);
	void remove_prim_all(primary_ptr p);
	bool fetch(string const &k,string &out,bool use_gzip);


public:
	thread_cache(int size=1) : limit(size)
	{
		pthread_mutex_init(&lru_lock,NULL);
		pthread_rwlock_init(&lock,NULL);
	};

	void set_size(int size) { limit=size; };
	virtual string insert(string const &key,string const &sec, string const &input,
			time_t timeout=365*24*3600,bool no_gzip=false);
	virtual bool fetch_string(string const &key,string &output);
	virtual bool fetch_gzip(string const &key,string &output);
	virtual void drop_primary(string const &key);
	virtual void drop_secondary(string const &key);
#ifdef CACHE_DEBUG
	void print_all();
#endif
	virtual ~thread_cache() {};
};

}

#endif
