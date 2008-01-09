#ifndef CACHE_H
#define CACHE_H

#include <pthread.h>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <string>
#include <list>
#include <map>

using boost::shared_ptr;
using boost::shared_array;
using namespace std; 

class Base_Cache {
protected:	
	string deflate(string const &s);
public:
	virtual string insert(string const &key, string const &input)
	{
		return deflate(input);
	};
	virtual bool fetch_string(string const &key,string &output) { return false; };
	virtual bool fetch_gzip(string const &key,string &output) { return false; };
	virtual void drop(string const &key) {};
	virtual void drop_prefix(string const &prefix) {};
	virtual ~Base_Cache(){};
};

class Memory_Cache : public Base_Cache 
{
	pthread_mutex_t lru_lock;
	pthread_rwlock_t lock;
	unsigned limit;
	struct Container {
		list<map<string,Container>::iterator>::iterator lru_ptr;
		string compressed;
		string text;
		Container(string c,
			  string const &s) : compressed(c), text(s) 
		{}; 
	};
	typedef map<string,Container> map_t;
	typedef map_t::iterator map_iterator_t;
	map_t data;
	list<map_iterator_t> lru;
	map_iterator_t fetch(string const &key);
public:	
	Memory_Cache(int size) : limit(size) 
	{
		pthread_mutex_init(&lru_lock,NULL);
		pthread_rwlock_init(&lock,NULL);
	};
	virtual string insert(string const &key, string const &input);
	virtual bool fetch_string(string const &key,string &output);
	virtual bool fetch_gzip(string const &key,string &output);
	virtual void drop(string const &key);
	virtual void drop_prefix(string const &prefix);
	virtual ~Memory_Cache() {};
};


extern shared_ptr<Base_Cache> global_cache;


#endif
