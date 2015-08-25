///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_CGI_API_H
#define CPPCMS_IMPL_CGI_API_H

#include <booster/noncopyable.h>
#include <booster/shared_ptr.h>
#include <booster/aio/buffer.h>
#include <booster/enable_shared_from_this.h>
#include <vector>
#include <map>
#include <booster/callback.h>
#include <booster/system_error.h>
#include <cppcms/http_context.h>

#include <cppcms/defs.h>
#include <cppcms/config.h>
#include "string_map.h"

namespace booster {
	namespace aio { 
		class io_service;
		class acceptor;
		class stream_socket;
	}
}


namespace cppcms {
	class service;
	class application;
	namespace http {
		class context;
		class request;
		class response;
	}


namespace impl {
	class multipart_parser;
	class cgi_filter;
namespace cgi {

	typedef booster::callback<void(booster::system::error_code const &e)> handler;
	typedef booster::callback<void(booster::system::error_code const &e,size_t)> io_handler;
	typedef booster::callback<void()> callback;
	typedef cppcms::http::context::handler ehandler;

	class connection;
	struct cgi_binder;
	typedef booster::intrusive_ptr<cgi_binder> cgi_binder_ptr;
	class acceptor : public booster::noncopyable {
	public:
		virtual void async_accept() = 0;
		virtual booster::aio::acceptor &socket() = 0;
		#ifndef CPPCMS_WIN32
		virtual booster::shared_ptr<cppcms::http::context> accept(int fd) = 0;
		#endif
		virtual void stop() = 0;
		virtual ~acceptor(){}
	};

	class CPPCMS_API connection : 
		public booster::noncopyable,
		public booster::enable_shared_from_this<connection>
	{
	public:
		connection(cppcms::service &srv);
		virtual ~connection();
		cppcms::service &service();
	
		void async_prepare_request(	http::context *context,
						ehandler const &on_post_data_ready);

		void async_write_response(	http::response &response,
						bool complete_response,
						ehandler const &on_response_written);

		void async_complete_response(	ehandler const &on_response_complete);

		void complete_response();
		
		void aync_wait_for_close_by_peer(callback const &on_eof);

		std::string getenv(std::string const &key);
		char const *cgetenv(char const *key);
		std::string getenv(char const *key);
		std::map<std::string,std::string> const &getenv();
		bool is_reuseable();

		/****************************************************************************/

		void async_write(void const *,size_t,io_handler const &h);
		size_t write(void const *,size_t,booster::system::error_code &e);

		booster::aio::io_service &get_io_service();
	private:

		cgi_filter &filter();
		size_t write_some(booster::aio::const_buffer const &b,booster::system::error_code &e);
		size_t read_some(booster::aio::mutable_buffer const &b,booster::system::error_code &e);
		void handle_io_readiness(booster::system::error_code const &ein,cgi_binder_ptr const &dt);
		void try_to_write_some(booster::system::error_code &e,cgi_binder_ptr const &dt);

		bool test_forwarding(char const *ptr,size_t len,cgi_binder_ptr const &dt);
		void on_headers_ready(cgi_binder_ptr const &dt,booster::system::error_code &e);
		bool on_some_input_read(char const *p,size_t n,cgi_binder_ptr const &dt);
		bool on_some_multipart_read(char const *p,size_t n,cgi_binder_ptr const &dt);

		void save_buffer(char const *ptr,size_t len);
		bool check_error(booster::system::error_code const &e,cgi_binder_ptr const &dt);
		void handle_input(char const *ptr,size_t len,cgi_binder_ptr const &dt);

		/****************************************************************************/

	protected:
		booster::shared_ptr<connection> self();

	private:

		struct reader;
		struct cgi_forwarder;
		struct async_write_binder;

		friend struct reader;
		friend struct writer;
		friend struct async_write_binder;
		friend struct cgi_forwarder;
		friend struct cgi_binder;
/*
		void set_error(ehandler const &h,std::string s);
		void on_headers_read(booster::system::error_code const &e,http::context *,ehandler const &h);
		void load_content(booster::system::error_code const &e,http::context *,ehandler const &h);
		void on_some_multipart_read(booster::system::error_code const &e,size_t n,http::context *,ehandler const &h);
		void on_async_write_written(booster::system::error_code const &e,bool complete_response,ehandler const &h);
		void on_eof_written(booster::system::error_code const &e,ehandler const &h);
		void handle_eof(callback const &on_eof);
		void handle_http_error(int code,http::context *context,ehandler const &h);
		void handle_http_error_eof(booster::system::error_code const &e,size_t n,int code,ehandler const &h); 
		void handle_http_error_done(booster::system::error_code const &e,int code,ehandler const &h);

		std::vector<char> content_;
		std::string async_chunk_;
		std::string error_;
		long long read_size_;
		std::auto_ptr<multipart_parser> multipart_parser_;

		*/

		cppcms::service *service_;
		bool request_in_progress_;
		booster::aio::stream_socket socket_;
		std::map<std::string,std::string> map_env_;

		size_t local_buffer_size_;
		std::vector<char> local_buffer_;

	};


} // cgi
} // impl
} // cppcms

#endif
