#ifndef CPPCMS_HTP_CONNECTION_SCGI_H
#define CPPCMS_HTP_CONNECTION_SCGI_H
#include "http_connection.h"
#include "asio_config.h"
namespace cppcms { namespace http {


template<typename Protocol>
class scgi_connection : public connection {
	friend template<T> class scgi_acceptor;
	aio::streambuf buf_;
	std::vector<char> data_;
	std::map<std::string,std::string> env_;
	aio::basic_stream_socket<Protocol> socket_;

	void on_done(error_code const &e,boost::fuction<void(bool>) callback)
	{
		callback(bool(e));
	}
	int check_size(aio::buffer &buf)
	{
		std::istream s(&buf);
		size_t n;
		s>>n;
		if(s.failbit())
			return -1;
		return n;
	}
	bool parse_env(std::vector<char> const &s)
	{
		if(s.back()!=',')
			return false;
		std::vector<char>::const_iterator b,e,p;
		b=s.begin(); e=s.begin()+s.size()-1;
		while(b!=e) {
			p=std::find(b,e,0);
			if(p==e)
				return false;
			std::string key(b,p);
			b=p+1;
			p=std::find(b,e,0);
			if(p==e)
				return false;
			std::string val(b,p);
			b=p+1;
			env_.insert(std::make_pair(key,val));
		}
		if(env_.find("CONTENT_LENGTH")==env_.end())
			return false;
		return true;
	}
public:
	scgi_connection(aio::io_service &s) :
		socket_(s)
	{
	}
	virtual bool keep_alive() { return false; }

	virtual size_t read(void *buffer,size_t s)
	{
		return aio::read(socket_,aio::buffer(buffer,s));
	}
	virtual size_t write(void const *buffer,size_t s)
	{
		return aio::write(socket_,aio::buffer(buffer,s));
	}
	virtual bool prepare()
	{
		try {
			aio::read_until(socket_,buf_,':');
			int n=check_size(buf);
			if(n<0)
				return false;
			data_.resize(n+1,0);
			if(aio::read(socket_,aio::buffer(data))!=n+1)
				return false;
			if(!parse_env(data))
				return false;

		}
		catch(std::exception const &e) {
			return false;
		}
		return true;
	}
	virtual void async_read(void *buffer,size_t s,boost::function<void(bool)> c)
	{
		aio::async_read(socket_,aio::buffer(buffer,s),
			boost::bind(&scgi_connection::on_done,shared_from_this(),aio::placeholders::error,c));
	}
	virtual void async_write(void const *buffer,size_t s,boost::function<void(bool)> c)
	{
		aio::async_write(socket_,aio::buffer(buffer,s),
			boost::bind(&scgi_connection::on_done,shared_from_this(),aio::placeholders::error,c));
	}
	virtual void async_prepare(boost::function<void(bool)> c)
	{
		aio::async_read_until(socket_,buf_,':',
			boost::bind(&scgi_connection::on_net_read,shared_from_this(),aio::placeholders::error,c));
	}
private:
	void on_net_read(error_code const &e,boost::function<void(bool)> c)
	{
		if(e) {
			c(false);
			return;
		}
		int n=check_size(buf);
		if(n<0) {
			c(false);
			return;
		}
		data_.resize(n+1,0);
		aio::async_read(socket_,aio::buffer(data_),
			boost::bind(&scgi_connection::on_env_read,aio::placeholders::error,c));
	}
	void on_env_read(error_code const &e,boost::function<void(bool)>)
	{
		bool res=true;
		if(e) {
			res=false;
		}
		if(res && !parse_env(data)) {
			res=false;
		}
		c(res);
	}
	void on_eof(error_code const &e,boost::function<void()> cb)
	{
		if(e) cb();
	}
public:
	void async_eof(boost::funcion<void()> cb)
	{
		static char tmp;
		aio::async_read(socket_,aio::buffer((void*)&tmp,1),
			boost::bind(&scgi_connection::on_eof,shared_from_this(),
			aio::placeholders::error,cb));
	}
	virtual std::string getenv(std::string const &key)
	{
		std::map<std::string,std::string>::iterator p;
		p=env_.find(key);
		if(p==env_.end())
			return std::string();
		return p->second;
	}

}; // connection_scgi

template<typename Protocol>
class scgi_acceptor : public async_acceptor {
	aio::basic_socket_acceptor<Protocol> socket_;
public:
	async_accptor(aio::io_service &srv) :
		socket_(srv)
	{
		socket_.assign(0); // Use default socket stdin
	}
	async_acceptor(aio::io_service &srv,aio::basic_endpoint<Protocol> const &end_point) :
		socket_(srv)
	{
		socket_.bind(end_point);
		socket_.listen();
	}
	virtual void async_accept(boost::shared_ptr<async_connection> conn,boost::function<void(bool)>)
	{
		boost::shared_ptr<scgi_connection<Protocol> > connection = 
			boost::dynamic_pointer_cast<scgi_connection<Protocol> >(conn);
		if(!connection)
			throw std::bad_cast();
		socket_.async_accept(connection->socket(),
			boost::bind(&scgi_acceptor::on_accepted,this,aio::placeholders::error));
	}

};



} } 
#endif
