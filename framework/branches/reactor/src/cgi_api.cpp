///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/url_dispatcher.h>
#include <cppcms/http_context.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
#include <cppcms/forwarder.h>
#include "http_protocol.h"
#include <cppcms/service.h>
#include "service_impl.h"
#include "cached_settings.h"
#include <cppcms/json.h>
#include "cgi_api.h"
#include "cgi_filter.h"
#include "multipart_parser.h"
#include <cppcms/util.h>
#include <scgi_header.h>
#include <stdlib.h>
#include <cppcms/config.h>
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/bind.hpp>
#else // Internal Boost
#   include <cppcms_boost/bind.hpp>
    namespace boost = cppcms_boost;
#endif

#include <booster/log.h>
#include <booster/aio/endpoint.h>
#include <booster/aio/aio_category.h>
#include <booster/aio/socket.h>
#include <booster/aio/buffer.h>


namespace cppcms { namespace impl { namespace cgi {

	//
	// Special forwarder from generic CGI to SCGI
	//
	/*
	struct connection::cgi_forwarder : public booster::enable_shared_from_this<connection::cgi_forwarder> {
	public:
		cgi_forwarder(booster::shared_ptr<connection> c,std::string ip,int port) :
			conn_(c),
			scgi_(c->get_io_service()),
			ep_(ip,port)
		{
			booster::aio::endpoint ep(ip,port);
			booster::system::error_code e;
			scgi_.open(ep.family(),e);
			if(e) {	return;	}
		}
		void async_run()
		{
			scgi_.async_connect(ep_,boost::bind(&cgi_forwarder::on_connected,shared_from_this(),_1));
		}
	private:
		void on_connected(booster::system::error_code const &e)
		{
			if(e) return;
			header_ = make_scgi_header(conn_->getenv(),0);
			scgi_.async_write(
				booster::aio::buffer(header_),
				boost::bind(&cgi_forwarder::on_header_sent,shared_from_this(),_1,_2));
		}
		void on_header_sent(booster::system::error_code const &e,size_t n)
		{
			if(e || n!=header_.size())
				return;
			header_.clear();
			std::string slen = conn_->getenv("CONTENT_LENGTH");
			content_length_ = slen.empty() ? 0LL : atoll(slen.c_str());
			if(content_length_ > 0) {
				post_.resize( content_length_ > 8192 ? 8192 : content_length_,0);
				write_post();
			}
			else {
				response_.resize(8192);
				read_response();
			}
		}
		void write_post()
		{
			if(content_length_ > 0) {
				if(content_length_ <  (long long)(post_.size())) {
					post_.resize(content_length_);
				}
				conn_->async_read_some(&post_.front(),post_.size(),
					boost::bind(&cgi_forwarder::on_post_data_read,shared_from_this(),_1,_2));
			}
			else {
				response_.swap(post_);
				response_.resize(8192);
				read_response();
			}
		}
		void on_post_data_read(booster::system::error_code const &e,size_t len)
		{
			if(e)  { cleanup(); return; }
			conn_->on_async_read_complete();
			scgi_.async_write(
				booster::aio::buffer(&post_.front(),len),
				boost::bind(&cgi_forwarder::on_post_data_written,shared_from_this(),_1,_2));
		}
		void on_post_data_written(booster::system::error_code const &e,size_t len)
		{
			if(e) { return; }
			content_length_ -= len;
			write_post();
		}
		
		void read_response() 
		{
			conn_->async_read_eof(boost::bind(&cgi_forwarder::cleanup,shared_from_this()));
			scgi_.async_read_some(booster::aio::buffer(response_),
						boost::bind(&cgi_forwarder::on_response_read,shared_from_this(),_1,_2));
		}
		void on_response_read(booster::system::error_code const &e,size_t len)
		{
			if(e) {
				conn_->async_write_eof(boost::bind(&cgi_forwarder::cleanup,shared_from_this()));
				return;
			}
			else {
				conn_->async_write(&response_.front(),len,boost::bind(&cgi_forwarder::on_response_written,shared_from_this(),_1,_2));
			}
		}
		void on_response_written(booster::system::error_code const &e,size_t )
		{
			if(e) { cleanup(); return; }
			scgi_.async_read_some(booster::aio::buffer(response_),
				boost::bind(&cgi_forwarder::on_response_read,shared_from_this(),_1,_2));
		}

		void cleanup()
		{
			booster::system::error_code e;
			scgi_.shutdown(booster::aio::stream_socket::shut_rdwr,e);
			scgi_.close(e);
		}

		booster::shared_ptr<connection> conn_;
		booster::aio::stream_socket scgi_;
		booster::aio::endpoint ep_;
		long long int content_length_;
		std::string header_;
		std::vector<char> post_;
		std::vector<char> response_;

	};
	*/





std::string connection::getenv(std::string const &key)
{
	return filter().env().get_safe(key.c_str());
}
char const *connection::cgetenv(char const *key)
{
	return filter().env().get_safe(key);
}
std::string connection::getenv(char const *key) 
{
	return filter().env().get_safe(key);
}
std::map<std::string,std::string> const &connection::getenv()
{
	string_map &e=filter().env();
	if(map_env_.empty() && e.begin()!=e.end()) {
		for(string_map::iterator p=e.begin();p!=e.end();++p) {
			map_env_[p->key]=p->value;
		}
	}
	return map_env_;
}


connection::connection(cppcms::service &srv) :
	service_(&srv),
	request_in_progress_(true),
	socket_(srv.get_io_service()),
	local_buffer_size_(0)
	#warning "Fix me - socket/io-service initialization"
{
}

connection::~connection()
{
}


cppcms::service &connection::service()
{
	return *service_;
}
booster::shared_ptr<connection> connection::self()
{
	return shared_from_this();
}

struct cgi_binder : public ehandler::callable_type {
	booster::shared_ptr<connection> conn;
	http::context *context;
	ehandler handler;
	cgi_binder(booster::shared_ptr<connection> c,http::context *ctx,ehandler const &h) :
		conn(c),
		context(ctx),
		handler(h)
	{
	}

	void handle(booster::system::error_code const &e)
	{
		if(e)
			handler(http::context::operation_aborted);
		else
			handler(http::context::operation_completed);
	}

	virtual void operator()(booster::system::error_code const &e)
	{
		conn->handle_io_readiness(e,this);
	}
};


size_t connection::write_some(booster::aio::const_buffer const &b,booster::system::error_code &e)
{
	// FIXME handle SSL
	return socket_.write_some(b,e);
}

size_t connection::read_some(booster::aio::mutable_buffer const &b,booster::system::error_code &e)
{
	// FIXME handle SSL
	return socket_.read_some(b,e);
}


void connection::handle_io_readiness(booster::system::error_code const &ein,cgi_binder_ptr const &dt)
{
	if(ein) {
		dt->handle(ein);
		return;
	}
	booster::system::error_code e;

	if(!filter().output.empty()) {
		size_t n = write_some(filter().output,e);
		filter().output += n;
		if(socket_.would_block(e) || (!e && !filter().output.empty())) {
			socket_.on_writeable(dt);
			return;
		}
		if(e) {
			dt->handle(e);
			return;
		}
	}
	if(local_buffer_size_ > 0) {
		size_t tmp = local_buffer_size_;
		local_buffer_size_=0;
		handle_input(&local_buffer_[0],tmp,dt);
	}
	else {
		size_t n = read_some(booster::aio::buffer(buffer_,buffer_len_),e);
		if(socket_.would_block(e)) 
			socket_.on_readable(dt);
		else if(e)
			dt->handle(e);
		else
			handle_input(buffer_,n,dt);
	}
}

void connection::save_buffer(char const *ptr,size_t len)
{
	if(len == 0)
		return;
	if(local_buffer_.size() < len)
		local_buffer_.resize(len);
	memmove(&local_buffer_[0],ptr,len);
	local_buffer_size_=len;
	local_buffer_.resize(len);
}

bool connection::check_error(booster::system::error_code const &e,cgi_binder_ptr const &dt)
{
	if(e) {
		if(filter().status() == cgi_filter::output_completed) {
			dt->status = e;
			return false;
		}
		dt->handle(e);
		return true;
	}
	return false;
}

void connection::handle_input(char const *ptr,size_t len,cgi_binder_ptr const &dt)
{
	for(;;) {
		booster::system::error_code e;
		if(!filter().output.empty()) {
			size_t n=write_some(filter().output,e);
			filter().output+=n;
			if(would_block(e)) {
				save_buffer(ptr,len);
				socket_.on_writeable(dt);
				return;
			}
			if(e) {
				dt->handle(e);
				return;
			}
			continue;
		}
		switch(filter().state()) {
		case cgi_filter::init:
			{
				size_t n = filter().consume_headers(ptr,len,e);
				ptr += n;
				len -=n;
				if(e) {
					dt->handle(e);
					return;
				}
				if(filter().state() >= cgi_filter::headers_ready) {
					if(test_forwarding(ptr,len,dt))
						return;
					on_headers_ready(dt,e);
					if(check_error(e,dt))
						return;
				}
			}
			break;
		case cgi_filter::headers_ready:
			{
				size_t n = filter().consume_body(ptr,len,e);
				ptr += n;
				len -=n;
				if(e) {
					dt->handle(e);
					return;
				}
				while(!filter().input.empty()) {
					const_buffer::buffer_data_type data = filter().input.get();
					for(size_t i=0;i<data.second;i++) {
						on_some_input_read(data.first[i].ptr,data.first[i].size,e);
						if(check_error(e))
							return;
						if(e) {
							filter().input.clear();
							break;
						}
					}
				}
			}
			break;
		case cgi_filter::body_read:
			{
				save_buffer(ptr,len);
				dt->handle(dt->status);
				return;
			}
			break;
		case cgi_filter::output_completed:
			save_buffer(ptr,len);
			dt->handle(dt->status);
			return;
		}
		if(len == 0 && filter().output.empty()) {
			socket_.on_readable(dt);
			return;
		}
	}
}

void connection::try_to_write_some(booster::system::error_code &e,
				  cgi_binder_ptr const &dt)
{
	while(!filter().output.empty()) {
		size_t n = socket_.write_some(filter().output,e);
		if(n > 0) {
			filter().output += n;
			continue;
		}
		if(e) {
			if(socket_.would_block(e)) {
				reading_=false;
				socket_.on_writeable(dt);
				return;
			}
			dt->handler(e);
			return;
		}
	}
}

bool connection::test_forwarding(char const *ptr,size_t len,cgi_binder_ptr const &dt)
{
	forwarder::address_type addr = service().forwarder().check_forwading_rules(
		cgetenv("HTTP_HOST"),
		cgetenv("SCRIPT_NAME"),
		cgetenv("PATH_INFO"));
	
	if(addr.second != 0 && !addr.first.empty()) {
		#warning "Fix me cgi_forwarder"
		/*booster::shared_ptr<cgi_forwarder> f(new cgi_forwarder(self(),addr.first,addr.second));
		f->set_data(ptr,len);
		f->async_run();*/
		dt->handler(http::context::operation_aborted);
		return true;
	}
	return false;
}

void connection::on_headers_ready(cgi_binder_ptr const &dt,booster::system::error_code &e)
{
	dt->context->request().prepare();
	
	http::content_type content_type = context->request().content_type_parsed();
	char const *s_content_length=cgetenv("CONTENT_LENGTH");
	long long content_length = *s_content_length == 0 ? 0 : atoll(s_content_length);

	if(content_length < 0)  {
		handle_http_error(400,e);
		return;
	}
	
	input_complete_= content_length == 0;
	if(content_length > 0) {
		if((is_multipart_ = (content_type.media_type()=="multipart/form-data"))) {
			// 64 MB
			long long allowed=service().cached_settings().security.multipart_form_data_limit*1024;
			if(content_length > allowed) { 
				BOOSTER_NOTICE("cppcms") << "multipart/form-data size too big " << content_length << 
					" REMOTE_ADDR = `" << getenv("REMOTE_ADDR") << "' REMOTE_HOST=`" << getenv("REMOTE_HOST") << "'";
				handle_http_error(413,context,h);
				return false;
			}
			multipart_parser_.reset(new multipart_parser(
				service().cached_settings().security.uploads_path,
				service().cached_settings().security.file_in_memory_limit));
			read_size_ = content_length;
			if(!multipart_parser_->set_content_type(content_type)) {
				BOOSTER_NOTICE("cppcms") << "Invalid multipart/form-data request" << content_length << 
					" REMOTE_ADDR = `" << getenv("REMOTE_ADDR") << "' REMOTE_HOST=`" << getenv("REMOTE_HOST") << "'";
				handle_http_error(400,e);
				return;
			}
		}
		else {
			long long allowed=service().cached_settings().security.content_length_limit*1024;
			if(content_length > allowed) {
				BOOSTER_NOTICE("cppcms") << "POST data size too big " << content_length << 
					" REMOTE_ADDR = `" << getenv("REMOTE_ADDR") << "' REMOTE_HOST=`" << getenv("REMOTE_HOST") << "'";
				handle_http_error(413,dt);
				return;
			}
			post_data_.resize(content_length);
			read_size_ = content_length;
		}
	}
	return ;
}



void connection::async_prepare_request(	http::context *context,
					ehandler const &h)
{
	socket_.on_readable(new cgi_binder(shared_from_this(),context,h));
}


void connection::aync_wait_for_close_by_peer(booster::callback<void()> const &on_eof)
{
	async_read_eof(boost::bind(&connection::handle_eof,self(),on_eof));
}

void connection::handle_eof(callback const &on_eof)
{
	if(request_in_progress_) {
		on_eof();
	}
}


void connection::handle_http_error(int code,booster::system::error_code &e)
{
	std::ostringstream ss;
	char const *status = http::response::status_to_string(code);
	ss.setlocale(std::locale::classic());
	if(context->service().cached_settings().service.generate_http_headers) {
		ss << "HTTP/1.0 " << code << ' ' << status 
		   << "\r\n"
		      "Connection: close\r\n"
		      "Content-Type: text/html\r\n"
		      "\r\n";
	}
	else {
		ss << "Content-Type: text/html\r\n"
		      "Status: " << code << ' ' << status << "\r\n"
		      "\r\n";
	}


	ss <<  "<html>\r\n"
		"<body>\r\n"
		"<h1>" << code << " " << status << "</h1>\r\n"
		"</body>\r\n"
		"</html>\r\n";
	
	async_chunk_ = ss.str();
	filter().format_output(async_chunk_.c_str(),async_chunk_.size(),true,e);
	if(!e) {
		e=FIXME;;
	}
}

void connection::handle_http_error_eof(
	booster::system::error_code const &e,
	size_t /*n*/,
	int code,
	ehandler const &h)
{
	if(e)  {
		set_error(h,e.message());
		return;
	}
	async_write_eof(boost::bind(&connection::handle_http_error_done,self(),_1,code,h));
}

void connection::handle_http_error_done(booster::system::error_code const &e,int code,ehandler const &h)
{
	if(e) {
		set_error(h,e.message());
		return;
	}
	set_error(h,http::response::status_to_string(code));
}

bool connection::on_some_input_read(char const *p,size_t n,cgi_binder_ptr const &dt)
{
	if(is_multipart_)
		return on_some_input_read(p,n,dt)
	
	if(n > read_size_) {
		handle_http_error(400,dt);
		return false;
	}
	size_t pos = post_data_.size() - read_size_;
	memcpy(&post_data_[read_size_],p,n);
	read_size_ -= n;
	return true;
}
bool connection::on_some_multipart_read(char const *p,size_t n,cgi_binder_ptr const &dt)
{
	read_size_-=n;
	if(read_size_ < 0) { 
		handle_http_error(400,dt); 
		return false;
	}
	multipart_parser::parsing_result_type r = multipart_parser_->consume(p,n);
	if(r == multipart_parser::eof) {
		if(read_size_ != 0)  {
			handle_http_error(400,dt);
			return false;
		}
		multipart_parser::files_type files = multipart_parser_->get_files();
		long long allowed=service().cached_settings().security.content_length_limit*1024;
		for(unsigned i=0;i<files.size();i++) {
			if(files[i]->mime().empty() && files[i]->size() > allowed) {
				BOOSTER_NOTICE("cppcms") << "multipart/form-data non-file entry size too big " << 
						files[i]->size() 
						<< " REMOTE_ADDR = `" << getenv("REMOTE_ADDR") 
						<< "' REMOTE_HOST=`" << getenv("REMOTE_HOST") << "'";
				handle_http_error(413,dt);
				return false;
			}
		}
		context->request().set_post_data(files);
		multipart_parser_.reset();
		return true;
	}
	else if (r==multipart_parser::parsing_error) {
		handle_http_error(400,dt);
		return true;
	}
	else if(r==multipart_parser::no_room_left) {
		handle_http_error(413,dt);
		return false;
	}
	else if(read_size_ == 0) {
		handle_http_error(400,dt);
		return false;
	}
	return true;
}


bool connection::is_reuseable()
{
	return error_.empty() && keep_alive();
}

std::string connection::last_error()
{
	return error_;
}

struct connection::async_write_binder : public booster::callable<void(booster::system::error_code const &,size_t)> {
	typedef booster::shared_ptr<cppcms::impl::cgi::connection> self_type;
	self_type self;
	ehandler h;
	bool complete_response;
	booster::shared_ptr<std::vector<char> > block;
	async_write_binder() :complete_response(false) {}
	void init(self_type c,bool comp,ehandler const &hnd,booster::shared_ptr<std::vector<char> > const &b) 
	{
		self=c;
		complete_response = comp;
		h = hnd;
		block = b;
	}
	void reset()
	{
		h=ehandler();
		self.reset();
		complete_response = false;
		block.reset();
	}
	void operator()(booster::system::error_code const &e,size_t)
	{
		self->on_async_write_written(e,complete_response,h);
		if(!self->cached_async_write_binder_) {
			self->cached_async_write_binder_ = this;
			reset();
		}
	}
};


void connection::async_write_response(	http::response &response,
					bool complete_response,
					ehandler const &h)
{
	http::response::chunk_type chunk = response.get_async_chunk();
	if(chunk.second > 0) {
		booster::intrusive_ptr<async_write_binder> binder;
		binder.swap(cached_async_write_binder_);
		if(!binder)
			binder = new async_write_binder();

		binder->init(self(),complete_response,h,chunk.first);
		booster::intrusive_ptr<booster::callable<void(booster::system::error_code const &,size_t)> > p(binder.get());
		async_write(	&(*chunk.first)[0],
				chunk.second,
				p);
		return;
	}
	if(!complete_response) {
		// request to send an empty block
		service().impl().get_io_service().post(boost::bind(h,http::context::operation_completed));
		return;
	}
	on_async_write_written(booster::system::error_code(),true,h); // < h will not be called when complete_response = true
}

void connection::on_async_write_written(booster::system::error_code const &e,bool complete_response,ehandler const &h)
{
	if(e) {	
		BOOSTER_WARNING("cppcms") << "Writing response failed:" << e.message();
		service().impl().get_io_service().post(boost::bind(h,http::context::operation_aborted));
		return;
	}
	if(complete_response) {
		async_write_eof(boost::bind(&connection::on_eof_written,self(),_1,h));
		request_in_progress_=false;
		return;
	}
	h(http::context::operation_completed);
}
void connection::async_complete_response(ehandler const &h)
{
	async_write_eof(boost::bind(&connection::on_eof_written,self(),_1,h));
	request_in_progress_=false;
}

void connection::complete_response()
{
	write_eof();
}

void connection::on_eof_written(booster::system::error_code const &e,ehandler const &h)
{
	if(e) { set_error(h,e.message()); return; }
	h(http::context::operation_completed);
}


struct connection::reader {
	reader(connection *C,io_handler const &H,size_t S,char *P) : h(H), s(S), p(P),conn(C)
	{
		done=0;
	}
	io_handler h;
	size_t s;
	size_t done;
	char *p;
	connection *conn;
	void operator() (booster::system::error_code const &e=booster::system::error_code(),size_t read = 0)
	{
		if(e) {
			h(e,done+read);
			return;
		}
		s-=read;
		p+=read;
		done+=read;
		if(s==0)
			h(booster::system::error_code(),done);
		else
			conn->async_read_some(p,s,*this);
	}
};

void connection::async_read(void *p,size_t s,io_handler const &h)
{
	reader r(this,h,s,(char*)p);
	r();
}

} // cgi
} // impl
} // cppcms
