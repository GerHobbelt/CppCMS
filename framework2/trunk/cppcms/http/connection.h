#ifndef CPPCMS_HTTP_CONNECTION_H
#define CPPCMS_HTTP_CONNECTION_H

#include <cppcms/defs.h>
#include <cppcms/util/noncopyable.h>
#include <cppcms/util/callback0.h>
#include <cppcms/util/callback1.h>

#include <string>

namespace cppcms { namespace http {

class CPPCMS_API connection : util::noncopyable {
public:
	virtual bool prepare() = 0;
	virtual std::string getenv(std::string const &key) = 0;
	virtual size_t read(void *buffer,size_t s) = 0;
	virtual size_t write(void const *buffer,size_t s) = 0;
	virtual ~connection()
};

class CPPCMS_API async_connection : public connection {
public:
	typedef ::cppcms::util::callback1<bool> cmp_handler;
	typedef ::cppcms::util::callback0 eof_handler;

	virtual bool keep_alive() = 0;
	virtual void async_prepare(cmp_handler) = 0;
	virtual void async_read_some(void *buffer,size_t s,cmp_handler) = 0;
	virtual size_t async_write_some(void const *buffer,size_t s,cmp_handler ) = 0;
	virtual void on_closed(eof_handler) = 0;
	virtual void cancel() = 0;
};



} } // cppcms::http

#endif

