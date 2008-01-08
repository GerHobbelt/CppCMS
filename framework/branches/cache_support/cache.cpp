#include "cache.h"
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <sstream>
#include <iostream>
#include <boost/format.hpp>

class RWLock {
protected:	
	pthread_rwlock_t *rwlock;
	RWLock(pthread_rwlock_t *p) : rwlock(p) { };
public:
	~RWLock() { pthread_rwlock_unlock(rwlock); };
};

class Write_Lock : public RWLock {
public:
	Write_Lock(pthread_rwlock_t *p) : RWLock(p) {pthread_rwlock_wrlock(p);};
};

class Read_Lock : public RWLock {
public:
	Read_Lock(pthread_rwlock_t *p) : RWLock(p) {pthread_rwlock_rdlock(p);};
};

class Mutex {
protected:	
	pthread_mutex_t *lock;
public:
	Mutex(pthread_mutex_t *p) : lock(p) { pthread_mutex_lock(p); };
	~Mutex() { pthread_mutex_unlock(lock); };
};

string Memory_Cache::insert(string const &key,string const &input)
{
	string result=deflate(input);
	Write_Lock L(&lock);
	map_iterator_t it=data.find(key);
	if(it==data.end() && data.size()>limit) {
		data.erase(*(lru.rbegin()));
		lru.pop_back();
	}
	if(it==data.end()) {
		data.insert(pair<string,Container>(key,Container(result,input)));
		it=data.find(key);
		lru.push_front(it);
		it->second.lru_ptr=lru.begin();
	}
	else {
		lru.erase(it->second.lru_ptr);
		lru.push_front(it);
		it->second.text=input;
		it->second.compressed=result;
	}
	return result;
}

Memory_Cache::map_iterator_t Memory_Cache::fetch( string const &key)
{
	map_iterator_t it=data.find(key);
	if(it==data.end()) {
		return it;
	}
	
	Mutex L(&lru_lock);
	lru.erase(it->second.lru_ptr);
	lru.push_front(it);
	return it;
}

bool Memory_Cache::fetch_string(string const &key,string &output)
{
	Read_Lock L(&lock);
	map_iterator_t it=fetch(key);
	if(it!=data.end()) {
		output=it->second.text;
		return true;
	}
	return false;
}

bool Memory_Cache::fetch_gzip(string const &key,string &output)
{
	Read_Lock L(&lock);
	map_iterator_t it=fetch(key);
	if(it!=data.end()) {
		output=it->second.compressed;
		return true;
	}
	return false;
}

void Memory_Cache::drop( string const &key)
{
	Write_Lock L(&lock);
	map_iterator_t it=data.find(key);
	if(it!=data.end()){
		lru.erase(it->second.lru_ptr);
		data.erase(it);
	}
}

void Memory_Cache::drop_prefix( string const &prefix)
{
	Write_Lock L(&lock);
	map_iterator_t it=data.lower_bound(prefix);
	while(it!=data.end() && it->first.find(prefix)==0){
		lru.erase(it->second.lru_ptr);
		map_iterator_t tmp=it;
		it++;
		data.erase(tmp);
	}
}


string Base_Cache::deflate(string const &text)
{
	using namespace boost::iostreams;
	
	ostringstream sstream;

	filtering_ostream zstream;
	zstream.push(gzip_compressor());
	zstream.push(sstream);
	zstream << text;
	zstream.pop();
	return sstream.str();
}
