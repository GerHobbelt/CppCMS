#ifndef BASE_CACHE_H
#define BASE_CACHE_H

#include <string>
#include <set>
#include <list>
#include "archive.h"

namespace cppcms {

using namespace std;

class base_cache {
public:
	virtual bool fetch_page(string const &key,string &output,bool gzip,time_t &timeout) = 0;
	virtual bool fetch(string const &key,archive &a,set<string> &tags,time_t &timeout) = 0;
	virtual int  rise(string const &trigger,list<string> *removed=NULL) = 0;
	virtual void clear() = 0;
	virtual void remove(string const &key) = 0;
	virtual void store(string const &key,set<string> const &triggers,time_t timeout,archive const &a) = 0;
	virtual void stats(unsigned &keys,unsigned &triggers) = 0;
	virtual ~base_cache() = 0;
};

}

#endif
