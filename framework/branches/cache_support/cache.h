#ifndef CACHE_H
#define CACHE_H

#include <pthread.h>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <string>

using boost::shared_ptr;
using boost::shared_array;
using namespace std; 

struct Compressed_Text {
	int length;
	shared_array<char> data;
};

class Base_Cache {
protected:	
	shared_ptr<Compressed_Text> deflate(string const &s);
public:
	virtual shared_ptr<Compressed_Text> insert(string const &key, string const &input)
	{
		return deflate(input);
	};
	virtual bool fetch_string(string const &key,string &output) { return false; };
	virtual bool fetch_gzip(string const &key,shared_array<Compressed_Text> &output) { return false; };
	virtual void drop(string const &key) {};
	virtual void drop_prefix(string const &prefix) {};
};

class Memory_Cache : public Base_Cache 
{
	pthread_muthex_t lru_lock;
	pthread_rwlock_t lock;
	int limit;
	struct Container {
		list<string>::iterator lru_ptr;
		shared_array<Compressed_Text> compressed;
		string text;
		Container(shared_array<Compressed_Text> c,
			  string const &s) : lru_ptr(l), compressed(c), text(s) 
		{}; 
	};
	typedef map<string,Container> map_t;
	typedef map_t::iterator map_iterator_t;
	map_t data;
	list<map_iterator_t> lru;
	map_iterator_t fetch(string const &key);
public:	
	virtual shared_ptr<Compressed_Text> insert(string const &key, string const &input);
	virtual bool fetch_string(string const &key,string &output);
	virtual bool fetch_gzip(string const &key,shared_ptr<Compressed_Text> &output);
	virtual void drop(string const &key);
	virtual void drop_prefix(string const &prefix);
};


#endif
