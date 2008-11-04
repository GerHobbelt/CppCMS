#include "base_cache.h"
namespace cppcms {

using namespace std;

bool base_cache::fetch_page(string const &key,string &output,bool gzip)
{
	return false;
}
bool base_cache::fetch(string const &key,archive &a,set<string> &tags,time_t &t)
{
	return false;
};

void base_cache::clear()
{
	// Nothing
}
int base_cache::rise(string const &trigger,list<string> *p)
{
	// Nothing
	return 0;
}

void base_cache::store(string const &key,set<string> const &triggers,time_t timeout,archive const &a)
{
	// Nothing
}

base_cache::~base_cache()
{
	// Nothing
}

void base_cache::remove(string const &t)
{
	// Nothing
}

void base_cache::stats(unsigned &keys,unsigned &triggers)
{
	keys=0;
	triggers=0;
}

}

