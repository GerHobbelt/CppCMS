#include "cache_interface.h"
#include "worker_thread.h"
#include "global_config.h"
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <sstream>
#include <iostream>

namespace cppcms {
using namespace std;

static string deflate(string const &text)
{
	using namespace boost::iostreams;
	gzip_params params;
	long level,length;

	if((level=global_config.lval("gzip.level",-1))!=-1){
		params.level=level;
	}		

	filtering_ostream zstream;

	if((length=global_config.lval("gzip.buffer",-1))!=-1){
		zstream.push(gzip_compressor(params,length));
	}
	else {
		zstream.push(gzip_compressor(params));
	}

	ostringstream sstream;

	zstream.push(gzip_compressor());
	zstream.push(sstream);
	zstream << text;
	zstream.pop();
	return sstream.str();
}


bool cache_iface::fetch_page(string const &key)
{
	string tmp;
	if(cms->caching_module->fetch_page(key,tmp,cms->gzip)) {
		cms->out=tmp;
		cms->gzip_done=true;
		return true;
	}
	return false;
}

void cache_iface::store_page(string const &key,time_t timeout)
{
	archive a;
	a<<(cms->out)<<deflate(cms->out);
	cms->caching_module->store(key,triggers,timeout,a);
}

void cache_iface::add_trigger(string const &t)
{
	triggers.insert(t);
}

void cache_iface::rise(string const &t)
{
	cms->caching_module->rise(t);
}

bool cache_iface::fetch_data(string const &key,serializable &data)
{
	archive a;
	set<string> new_trig;
	if(cms->caching_module->fetch(key,a,new_trig)) {
		data.load(a);
		triggers.insert(new_trig.begin(),new_trig.end());
		return true;
	}
	return false;
}

void cache_iface::store_data(string const &key,serializable const &data,
			set<string> const &triggers,
			time_t timeout)
{
	archive a;
	data.save(a);
	this->triggers.insert(triggers.begin(),triggers.end());
	cms->caching_module->store(key,triggers,timeout,a);
}

bool cache_iface::fetch_frame(string const &key,string &result)
{
	archive a;
	set<string> new_trig;
	if(cms->caching_module->fetch(key,a,new_trig)) {
		a>>result;
		triggers.insert(new_trig.begin(),new_trig.end());
		return true;
	}
	return false;
}

void cache_iface::store_frame(string const &key,string const &data,
			set<string> const &triggers,
			time_t timeout)
{
	archive a;
	a<<data;
		
	this->triggers.insert(triggers.begin(),triggers.end());
	cms->caching_module->store(key,triggers,timeout,a);
}


} // End of namespace cppcms
