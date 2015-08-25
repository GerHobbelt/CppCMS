///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////

#include "cgi_filter.h"

namespace cppcms {
namespace impl {

class scgi_filter : public cgi_filter {
public:
	scgi_filter() : 
		headers_length_(0),
		got_len_(false),
		reading_key_(true),
		content_length_(-1),
		consumed_content_(0)
	{
	}
	virtual bool   keep_alive_supported()
	{
		return false;
	}
	virtual size_t consume_headers(char const *data,size_t len,booster::system::error_code &e) 
	{
		size_t saved_len = len;
		if(!e && !got_len_) {
			while(len > 0) {
				char c = *data++;
				len--;
				if(c == ':') {
					got_len_ = true;
					pool_.set_page_size(len);
					break;
				}
				if(c<'0' || '9'<c) {
					e=vilotation();
					break;
}
				headers_length_=headers_length_*10 + (c-'0');
			}
			if(len > 65535) {
				e=vilotation();
				break;
			}
		}
		while(!e && headers_length_ > 0 && len > 0) {
			char c=*data++;
			len--;
			headers_length_--;
			if(c!=0) {
				if(reading_key_)
					key_ += c;
				else
					value_+= c;
			}
			else {
				if(reading_key_) {
					reading_key_ =false;
				}
				else {
					if(key_ == "CONTENT_LENGTH") {
						long long cl = atoll(value_.c_str());
						if(content_length_!=-1 || cl<0) {
							e=violation();
							breakl
						}
						content_length_ = cl;
					}
					setenv(key_.c_str(),value_.c_str());
					reading_key_=true;
					key_.resize(0);
					value_resize(0);
				}
			}
		}
		if(!e && headers_length_ == 0 && len > 0) {
			char c=*data++;
			len--;
			if(c==',') {
				if(content_length_ != 0)
					state(headers_ready);
				else
					state(body_read);
			}
			else
				e=violation();
		}
		return saved_len - len;
	}

	virtual size_t consume_body(char const *data,size_t len,booster::system::error_code &e) 
	{
		if(consumed_content_ + (long long)(len) > content_length_) {
			e=violation();
			len = content_length_ - consumed_content_;
		}
		consumed_content_ += len;
		input = booster::aio::buffer(data,len);
		if(consumed_content_ == content_length_)
			state(body_read); 
		return len;
	}
	virtual size_t format_output(char const *data,size_t len,bool eof,booster::system::error_code &e)
	{
		output = booster::aio::buffer(data,len);
		if(eof)
			state(output_completed);
		else
			state(sending_content);
		return len;
	}
private:
	int headers_length_;
	bool got_len_;
	bool reading_key_;
	long long content_length_;
	long long consumed_content_;

	std::string key_,value_;
}

} // impl
} // cppcms
