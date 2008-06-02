#ifndef CACHE_IFACE_H
#define CACHE_IFACE_H

namespace cppcms {

class worker_thread;
class cache_iface {
	worker_thread *w;
	set<string> triggers;
public:
	reset() { triggers.erase() };
	cache_iface(worker_thread *w) : cms (w) {};
	bool fetch_page(string const &key);
	void store_page(string const &key,time_t timeout=TIME_T_MAX);
	void rise(string const &trigger);
	void add_trigger(string const &trigger);
	bool fetch_frame(string const &key,string &result);
	void store_frame(string const &key,
			 string const &frame,
			 set<string> const &triggers=set<string>(),
			 time_t timeout=INFTY)
	bool fetch_data(string const &key,serializable &data);
	void store_data(string const &key,serializable const &data,
			set<string> const &triggers=set<string>(),
			timeout_t timeout=INFTY);
	
};

}

#endif
