#include "cache_iface.h"

bool cache_iface::fetch_page(string const &key)
{
	string tmp;
	if(cms->base_cache->fetch_page(key,tmp,cms->gzip)) {
		out=tmp;
		cms->gzip_done=true;
		return true;
	}
	return false;
}

void cache_iface::store_page(string const &key,time_t timeout)
{
	archive a;
	page p(out);
	p.save(a);
	cms->base_cache->store(key,triggers,timeout,a);
}

void cache_iface::add_trigger(string const &t)
{
	triggers.insert(t);
}

void cache_iface::rise(string const &t)
{
	cms->base_cache->rise(t);
}

bool cache_iface::fetch_data(string const &key,serializable &data)
{
	archive a;
	set<string> new_trig;
	if(cms->base_cache->fetch(key,a,new_trig)) {
		data.load(a);
		triggers.insert(new_trig.begin(),new_trig,end());
		return true;
	}
	return false;
}

void cache_iface::store_data(string const &key,serializable const &data,
			set<string> const &triggers=set<string>(),
			timeout_t timeout)
{
	archive a;
	data.save(a);
	this->triggers.insert(triggers.begin(),triggers,end());
	cms->base_cache->store(key,triggers,timeout,a);
}

bool cache_iface::fetch_frame(string const &key,string &result)
{
	archive a;
	set<string> new_trig;
	if(cms->base_cache->fetch(key,a,new_trig)) {
		a>>result;
		triggers.insert(new_trig.begin(),new_trig,end());
		return true;
	}
	return false;
}

void cache_iface::store_frame(string const &key,string const &data,
			set<string> const &triggers=set<string>(),
			timeout_t timeout)
{
	archive a;
	a<<data;
		
	this->triggers.insert(triggers.begin(),triggers,end());
	cms->base_cache->store(key,triggers,timeout,a);
}
