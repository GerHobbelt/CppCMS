#include "config.h"
#ifdef __CYGWIN__
// Cygwin ASIO works only with win32 sockets
#define _WIN32_WINNT 1
#define __USE_W32_SOCKETS 1
#endif

#ifdef USE_BOOST_ASIO
#include <boost/asio.hpp>
namespace aio = boost::asio;
using boost::system::error_code;
using boost::system::system_error;
#else
#include <asio.hpp>
namespace aio = asio;
using asio::error_code;
using asio::system_error;
#endif

#include "tcp_cache.h"
#include "tcp_cache_protocol.h"
#include "archive.h"

using aio::ip::tcp;

namespace cppcms {


class l1_cleanup_connection : boost::noncopyable : boost::enable_shared_from_this<l1_cleanup_connection> {
	aio::io_service &srv_
	tcp::socket s_
	string ip;
	int port;
	base_cache *cache;
	cache_factory *cf;

	aio::streambuf buf;

	void connect(string ip,int port) {
		ip_=ip;
		port_=port;
		error_code e;
		s_.connect(tcp::endpoint(aio::ip::address::from_string(ip),port),e);
		if(e) throw cppcms_error("connect:"+e.message());
		tcp::no_delay nd(true);
		socket_.set_option(nd);
	}
public:
	l1_cleanup_connection(aio::io_service &srv,string _ip,int _port,cache_factory *f) :
		srv_(srv),
		s_(srv),
		ip(_ip),
		port(_port+1),
		cf(f)
	{
		cache=cf->get();
		connect(ip,port);
	}
	~l1_cleanup_connection() 
	{
		cf->del(cache);
	}
	void run(error_code const &e=error_code())
	{
		if(e) {
			cache->clear();
			s_.close();
			s_.async_connect(tcp::endpoint(aio::ip::address::from_string(ip),port),
				boost::bind(&l1_cleanup_connection::run,shared_from_this(),_1));
			return;
		}
		asio::async_read_untill(s_,buf,'\n',
			boost::bind(&l1_cleanup_connection::exec,shared_from_this(),_1));
	}
	void exec(error_code const &e)
	{
		if(e) {
			run(e)
			return;
		}
		istream is(&buf);
		string cmd;
		getline(is,cmd);
		if(cmd=="clear:") {
			cache->clear();
		}
		else if(cmd.size()>5 && memcmp(cmd.c_str(),"drop:")==0) {
			cache->remove(cmd.substr(5));
		}
		run();
	}
};

class l1_cleanup_service : boost::noncopyable {
	aio::io_service srv;
	pthread_t pid;

	static void *thread_func(void *self_ptr)
	{
		l1_cleanup_service &self=*(l1_cleanup_service*)self_ptr;
		try {
			self.srv.run()
		}
		catch(std::exception &e) {
			std::cerr<<e.what()<<endl;
		}
		catch(...) {}
		return NULL;
	}
	
public:
	l1_cleanup_service(vector<string> const &ips,vector<long> const &ports) 
	{
		unsigned i;
		for(i=0;i<ips.size() && i<ports.size();i++) {
			shared_ptr<l1_cleanup_connection> 
				ptr(new l1_cleanup_connection(srv,ips[i],ports[i],cache_factory));
			ptr->run();
		}
		pthread_create(&pid,NULL,l1_cleanup_service::thread_func,this);
	}
	~l1_cleanup_service()
	{
		try {
			srv.stop();
			pthread_join(pid,NULL);
		}
		catch(...){}
	}
};

class messenger : boost::noncopyable {
	aio::io_service srv_;
	tcp::socket socket_;
	string ip_;
	int port_;
public:
	void connect(string ip,int port) {
		ip_=ip;
		port_=port;
		error_code e;
		socket_.connect(tcp::endpoint(aio::ip::address::from_string(ip),port),e);
		if(e) throw cppcms_error("connect:"+e.message());
		tcp::no_delay nd(true);
		socket_.set_option(nd);
	}
	messenger(string ip,int port) :
		socket_(srv_)
	{
		connect(ip,port);
	}
	messenger() : socket_(srv_) { };

	void transmit(tcp_operation_header &h,string &data)
	{
		bool done=false;
		int times=0;
		do {
			try {
				aio::write(socket_,aio::buffer(&h,sizeof(h)));
				if(h.size>0) {
					aio::write(socket_,aio::buffer(data,h.size));
				}
				aio::read(socket_,aio::buffer(&h,sizeof(h)));
				if(h.size>0) {
					vector<char> d(h.size);
					aio::read(socket_,aio::buffer(d,h.size));
					data.assign(d.begin(),d.begin()+h.size);
				}
				done=true;
			}
			catch(system_error const &e) {
				if(times) {
					throw cppcms_error(string("tcp_cache:")+e.what());
				}
				socket_.close();
				error_code er;
				socket_.connect(
					tcp::endpoint(
						aio::ip::address::from_string(ip_),port_),er);
				if(er) throw cppcms_error("reconnect:"+er.message());
				times++;
			}
		}while(!done);
	}
	
};

tcp_cache::tcp_cache(vector<string> const& ip,vector<long> const &port,cache_factory *cf)
{
	L1=NULL;
	L1_factory=cf;
	if(ip.size()<1 || port.size()!=ip.size()) {
		throw cppcms_error("Incorrect parameters for tcp cache");
	}
	conns=ip.size();
	tcp=new messenger[conns];
	try {
		for(int i=0;i<conns;i++) {
			tcp[i].connect(ip[i],port[i]);
		}
		if(cf)	L1=cf->get();
	}
	catch(...) {
		delete [] tcp;
		tcp=NULL;
		if(L1 && cf) cf->del(L1);
		throw;
	}
}

tcp_cache::~tcp_cache()
{
	delete [] tcp;
	if(L1) L1_factory->del(L1);
}

void tcp_cache::broadcast(tcp_operation_header &h,string &data)
{
	int i;
	for(i=0;i<conns;i++) {
		tcp_operation_header ht=h;
		string dt=data;
		tcp[i].transmit(ht,data);
	}
}

int tcp_cache::rise(string const &trigger,list<string> *l)
{
	tcp_operation_header h={0};
	h.opcode=opcodes::rise;
	h.size=trigger.size();
	string data=trigger;
	h.operations.rise.trigger_len=trigger.size();
	broadcast(h,data);
	return 0;
}

void tcp_cache::clear()
{
	tcp_operation_header h={0};
	h.opcode=opcodes::clear;
	h.size=0;
	string empty;
	broadcast(h,empty);
}

bool tcp_cache::base_fetch(string const &key,archive &a,set<string> &tags,time_t &t)
{
	string data=key;
	tcp_operation_header h={0};
	h.opcode=opcodes::fetch;
	h.size=data.size();
	h.operations.fetch.key_len=data.size();
	get(key).transmit(h,data);
	if(h.opcode!=opcodes::data)
		return false;
	char const *ptr=data.c_str();
	a.set(ptr,h.operations.data.data_len);
	t=h.operations.data.timeout;
	ptr+=h.operations.data.data_len;
	int len=h.operations.data.triggers_len;
	while(len>0) {
		string tag;
		unsigned tmp_len=strlen(ptr);
		tag.assign(ptr,tmp_len);
		ptr+=tmp_len+1;
		len-=tmp_len+1;
		tags.insert(tag);
	}
	return true;
}

bool tcp_cache::l1_fetch_page(string const  &key,string &output,bool gzip)
{
	static char const *no_gzip_key="CPPCmsAbRAKaDABRA";
	if(L1->fetch_page(key,output,gzip)) {
		if(gzip)
			return true;
		else if(output!=no_gzip_key)
			return true;
		L1->remove(key);
	}
	archive &a;
	set<string> &tags;
	time_t timeout;

	if(!gzip) {
		if(!base_fetch(key,a,tags,timeout)) {
			return false;
		}
		L1->store(key,a,tags,timeout);
		a>>output;
	}
	else {
		if(!base_fetch_page(key,output,timeout))
			return false;
		a<<string(no_gzip_key)<<output;
		L1->store(key,a,tags,timeout);
	}
	return true;
}

bool tcp_cache::base_fetch_page(string const  &key,string &output,bool gzip,time_t &timeout)
{
	string data=key;
	tcp_operation_header h={0};
	h.opcode=opcodes::fetch_page;
	h.size=data.size();
	h.operations.fetch_page.gzip=gzip;
	h.operations.fetch_page.strlen=data.size();
	get(key).transmit(h,data);
	if(h.opcode==opcodes::page_data) {
		output=data;
		timeout=h.operations.page_data.timeout;
		return true;
	}
	return false;
}

bool tcp_cache::fetch_page(string const  &key,string &output,bool gzip,time_t &timeout)
{
	if(L1) return l1_fetch_page(key,output,gzip);
	return base_fetch_page(key,output,gzip,timeout);
}

bool tcp_cache::fetch(string const &key,archive &a,set<string> &tags,time_t &t)
{
	if(L1 && L1->fetch(key,a,tags,t)) {
		return true;
	}
	if(!base_fetch(key,a,tags,t)) {
		return false;
	}
	if(L1) L1->store(key,a,tags,t);
	return true;
}

void tcp_cache::remove(string const &s)
{
	// NOT USED
}

void tcp_cache::stats(unsigned &keys,unsigned &triggers)
{
	keys=0; triggers=0;
	for(int i=0;i<conns;i++) {
		tcp_operation_header h={0};
		string data;
		h.opcode=opcodes::stats;
		tcp[i].transmit(h,data);
		if(h.opcode==opcodes::out_stats) {
			keys+=h.operations.out_stats.keys;
			triggers+=h.operations.out_stats.triggers;
		}
	}
}

void tcp_cache::store(string const &key,set<string> const &triggers,time_t timeout,archive const &a)
{
	tcp_operation_header h={0};
	string data;
	h.opcode=opcodes::store;
	data.append(key);
	h.operations.store.key_len=key.size();
	data.append(a.get());
	h.operations.store.data_len=a.get().size();
	time_t now;
	time(&now);
	h.operations.store.timeout=timeout-now > 0 ? timeout-now : 0;
	unsigned tlen=0;
	for(set<string>::const_iterator p=triggers.begin(),e=triggers.end();p!=e;++p) {
		tlen+=p->size()+1;
		data.append(p->c_str(),p->size()+1);
	}
	h.operations.store.triggers_len=tlen;
	h.size=data.size();
	get(key).transmit(h,data);
}

messenger &tcp_cache::get(string const &key)
{
	if(conns==1) return tcp[0];
	unsigned val=0,i;
	for(i=0;i<key.size();i++) {
		val+=251*key[i]+103 % 307;
	}
	return tcp[val % conns];
}

tcp_cache_factory::tcp_cache_factory(vector<string> const &_ip,vector<long> const &_port,auto_ptr<cache_factory> l1) :
		ip(_ip),
		port(_port),
		L1_factory(l1)
{
	if(!L1_factory.get())
		return;
	cleanup=auto_ptr<l1_cleanup_service>(new l1_cleanup_service(_ip,_port));
	cleanup->run();
}
base_cache *tcp_cache_factory::get() const
{
	return new tcp_cache(ip,port,l1.get());
};
void tcp_cache_factory::del(base_cache *p) const
{
	delete p;
};

tcp_cache_factory::~tcp_cache_factory()
{
	cleanup->stop();
}

}


#ifdef TCP_CACHE_UNIT_TEST

#include <assert.h>
#include <iostream>
#include <cstdlib>
int main(int argc,char **argv)
{
	using namespace cppcms;
	using namespace std;
	if(argc!=3) {
		cerr<<"Usage IP port"<<endl;
		return 1;
	}
	try {
		archive a;
		set<string> s;
		time_t tin;
		tcp_cache tcp(argv[1],atoi(argv[2]));
		assert(tcp.fetch("something",a,s,tin)==false);
		time_t t;
		time(&t);
		t+=2;
		a.set("data",4);
		tcp.store("key",s,t,a);
		unsigned keys,triggers;
		tcp.stats(keys,triggers);
		assert(keys==1);
		assert(triggers==1);
		s.clear();
		a.set("");
		assert(tcp.fetch("key",a,s,tin)==true);
		assert(s.size()==1);
		assert(*(s.begin())=="key");
		assert(a.get()=="data");
		sleep(3);
		assert(tcp.fetch("key",a,s,tin)==false);
		a.set("");
		a<<string("msg1");
		a<<string("msg2");
		time(&t);
		t+=50;
		s.clear();
		s.insert("a");
		s.insert("b");
		tcp.store("k",s,t,a);
		string x;
		assert(tcp.fetch_page("k",x,true)==true);
		assert(x=="msg2");
		assert(tcp.fetch_page("k",x,false)==true);
		assert(x=="msg1");
		a.set("");
		s.clear();
		assert(tcp.fetch("k",a,s,tin)==true);
		assert(s.size()==3);
		set<string>::iterator ptr=s.begin();
		assert(*ptr++=="a");
		assert(*ptr++=="b");
		assert(*ptr++=="k");
		tcp.rise("a");
		assert(tcp.fetch("k",a,s,tin)==false);
		a.set("Something");
		s.clear();
		tcp.store("bb",s,t,a);
		assert(tcp.fetch("xx",a,s,tin)==false);
		assert(tcp.fetch("bb",a,s,tin)==true);
		tcp.clear();
		assert(tcp.fetch("bb",a,s,tin)==false);
		cout<<"Done... OK!\n";
	}
	catch(std::exception const &e) {
		cerr<<e.what()<<endl;
	}
}

#endif
