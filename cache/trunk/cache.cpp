#include "cache.h"
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <sstream>
#include <iostream>
#include <boost/format.hpp>

namespace cache {

string base_cache::deflate(string const &text)
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


class mutex_lock {
	pthread_mutex_t &m;
public:
	mutex_lock(pthread_mutex_t &p): m(p) { pthread_mutex_lock(&m); };
	~mutex_lock() { pthread_mutex_unlock(&m); };
};

class rwlock_rdlock {
	pthread_rwlock_t &m;
public:
	rwlock_rdlock(pthread_rwlock_t &p): m(p) { pthread_rwlock_rdlock(&m); };
	~rwlock_rdlock() { pthread_rwlock_unlock(&m); };
};

class rwlock_wrlock {
	pthread_rwlock_t &m;
	public:
	rwlock_wrlock(pthread_rwlock_t &p): m(p) { pthread_rwlock_wrlock(&m); };
	~rwlock_wrlock() { pthread_rwlock_unlock(&m); };
};


bool thread_cache::fetch(string const &key,string & out,bool use_gzip)
{
	bool res;

	rwlock_rdlock dolock(lock);

	primary_ptr p=primary.find(key);
	if(p==primary.end() || p->second.time_entry->first < time(NULL)) {
		res=false;
	}
	else {
		res=true;
		if(!use_gzip) {
			out=p->second.original;
		}
		else if (p->second.has_gzipped){
			out=p->second.gzipped;
		}
		else {
			res=false;
		}
		if(res) {
			mutex_lock m(lru_lock);

			lru.erase(p->second.lru_entry);
			lru.push_front(p);
			p->second.lru_entry=lru.begin();
		}
	}

	return res;
}

void thread_cache::remove_sec_by_prim(string const &key)
{
	prim2sec_ptr ps=prim2sec.find(key);
	while(ps!=prim2sec.end() && ps->first==key) {
		sec2prim.erase(ps->second);
		prim2sec_ptr ps_tmp=ps;
		prim2sec.erase(ps_tmp);
		ps++;
	}
}

void thread_cache::remove_prim(primary_ptr p)
{
	if(p!=primary.end()) {
		lru.erase(p->second.lru_entry);
		timeout.erase(p->second.time_entry);
		primary.erase(p);
	}
}

void thread_cache::remove_prim_all(primary_ptr p)
{
	remove_sec_by_prim(p->first);
	remove_prim(p);
}

void thread_cache::drop_secondary(string const &k)
{
	rwlock_wrlock l(lock);
	sec2prim_ptr p=sec2prim.find(k);
	while(p!=sec2prim.end() && p->first==k){
		sec2prim_ptr p_tmp=p;
		p++;
		prim2sec.erase(p_tmp->second->first);
		remove_prim(p_tmp->second);
		sec2prim.erase(p_tmp);
	}
}

void thread_cache::drop_primary(string const &key)
{
	rwlock_wrlock l(lock);
	remove_prim(primary.find(key));
	remove_sec_by_prim(key);
}

void thread_cache::insert(container &c, string const &k,vector<string> const &secondary,time_t lifetime)
{
	rwlock_wrlock l(lock);
	primary_ptr the_ptr=primary.end();
	if(primary.size()>limit && (the_ptr=primary.find(k))==primary.end()) {
		multimap<time_t,primary_ptr>::iterator p=timeout.begin();
		if(p->first < time(NULL)) {
			remove_prim_all(p->second);
		}
		else {
			remove_prim_all(lru.back());
		}
	}
	if(the_ptr!=primary.end()) {
		the_ptr->second.has_gzipped=c.has_gzipped;
		the_ptr->second.original=c.original;
		the_ptr->second.gzipped=c.gzipped;
		timeout.erase(the_ptr->second.time_entry);
		remove_sec_by_prim(k);
	}
	else {
		pair<primary_ptr,bool> res=primary.insert(pair<string,container>(k,c));
		the_ptr=res.first;
		lru.push_front(the_ptr);
		the_ptr->second.lru_entry=lru.begin();
	}
	the_ptr->second.time_entry=timeout.insert(
			pair<time_t,primary_ptr>(time(NULL)+lifetime,the_ptr));
	unsigned i;
	for(i=0;i<secondary.size();i++) {
		sec2prim_ptr tmp=sec2prim.insert(make_pair(secondary[i],the_ptr));
		prim2sec.insert(make_pair(k,tmp));
	}
}

bool thread_cache::fetch_string(string const &key,string &out)
{
	return fetch(key,out,false);
}
bool thread_cache::fetch_gzip(string const &key,string &out)
{
	return fetch(key,out,true);
}

void base_cache::split_to_keys(string const &s,vector<string> &v)
{
	unsigned n=0;
	unsigned i;
	for(i=0;i<s.size();i++) {
		if(s[i]==',') n++;
	}
	v.reserve(n);
	size_t p=0,pn=0;
	for(i=0;i<n;i++){
		pn=s.find(',',p);
		string tmp=s.substr(p,pn-p);
		if(tmp!="")
			v.push_back(tmp);
		p=pn+1;
	}
	v.push_back(s.substr(p,s.size()-p));
}

string thread_cache::insert(string const &key,string const &sec, string const &input,time_t timeout,bool no_gzip)
{
	container c;
	c.original=input;
	if(no_gzip) {
		c.has_gzipped=false;
	}
	else {
		c.gzipped=deflate(input);
		c.has_gzipped=true;
	}
	vector<string> skeys;
	split_to_keys(sec,skeys);
	insert(c,key,skeys,timeout);
	if(!no_gzip)
		return c.gzipped;
	return "";
}

#ifdef CACHE_DEBUG

void thread_cache::print_all()
{
	primary_ptr p;
	for(p=primary.begin();p!=primary.end();p++) {
		cout<<"Key:"<<p->first<<":"<<p->second.original<<endl;
	}
	container::lru_ptr pl;
	for(pl=lru.begin();pl!=lru.end();pl++) {
		cout<<"Lru:"<<(*pl)->first<<endl;
	}
	container::timeout_ptr pt;
	for(pt=timeout.begin();pt!=timeout.end();pt++) {
		cout<<"time:"<<pt->second->first<<":"<<(pt->first-time(NULL))<<endl;
	}
}

#endif
}
