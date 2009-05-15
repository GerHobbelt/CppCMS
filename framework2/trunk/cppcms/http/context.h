#ifndef CPPCMS_HTTP_CONTEXT_H
#define CPPCMS_HTTP_CONTEXT_H

#include <cppcms/defs.h>
#include <cppcms/utils/noncopyable.h>
#include <memory>

namespace cppcms {
	class locale_interface;
namespace http {
	
	class request;
	class response;
	class connection;

	class context_impl;
	class CPPCMS_API context : public util::noncopyable {
		std::auto_ptr<context_impl> impl_;
		std::auto_ptr<application> app_;
	public:
		http::request &request();
		http::response &response();
		http::connection &connection();
		locale_interface &locale();

	public: /* For Internal Use */

		context();
		~context();
	};


} // http
} // cppcms



#endif
