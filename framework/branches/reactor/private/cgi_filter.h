///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2015  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_PRIVATE_CGI_FILTER_H
#define CPPCMS_PRIVATE_CGI_FILTER_H
#include "cppcms_error_category.h"
#include <booster/system_error.h>
#include <booster/aio/buffer.h>
namespace cppcms {
namespace impl {

class cgi_filter {
public:
	
	enum cgi_state{
		init,
		headers_ready,
		body_read,
		output_completed
	};

	booster::aio::const_buffer output;
	booster::aio::const_buffer input;
	
	virtual bool   is_reuseable() = 0;
	virtual size_t consume_headers(char const *data,size_t len,booster::system::error_code &e);
	virtual size_t consume_body(char const *data,size_t len,booster::system::error_code &e);
	virtual size_t consume_input(void const *data,size_t len,booster::system::error_code &e) = 0;
	virtual size_t format_output(void const *data,size_t len,bool eof,booster::system::error_code &e) = 0;

	
	string_map &env()
	{
		return env_;
	}

	cgi_state state()
	{
		return state_;
	}

	void state(cgi_state s)
	{
		state_ = s;
		if(s == headers_ready)
			env_.sort();

	}

protected:
	cgi_filter()  
		: state_(init)
	{

	}
	booster::system::error_code violation()
	{
		return booster::system::error_code(errc::protocol_violation,cppcms_category);
	}

	void setenv(char const *key,char const *value)
	{
		key = pool_.add(key);
		value = pool_.add(value);
		env_.add(key,value);
	}
	void setenv(char const *key,int klen,char const *value,int vlen)
	{
		key = pool_.add(key,key+klen);
		value = pool_.add(value,value+vlen);
		env_.add(key,value);
	}

	cgi_state state_;
	string_pool pool_;
	string_map env_;

};

} // impl
} // cppcms
#endif
