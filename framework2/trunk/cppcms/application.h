#ifndef CPPCMS_APPICATION_H
#define CPPCMS_APPICATION_H

#include <cppcms/defs.h>
#include <cppcms/util/noncopyable.h>

#include <string>

namespace cppcms {
	namespace http {
		class request;
		class response;
		class context;
		class view_interface;
	} // http

	class cache_interface;
	class session_interface;
	class locle_interface;
	class url_dispatcher;
	class view_interface;

	class CPPCMS_API application : public util::noncopyable {
	public:
		http::context &context();
		http::request &request();
		http::response &response();
		cache_interface &cache();
		session_interface &session();
		locale_interface &locale();
		view_interface &view();
		url_dispatcher &url();
	
	// public virtual functions
		virtual void run(http::context &context,std::string path);

	// ctor/dtor
		application();
		virtual ~application();
	};


} // cppcms


#endif
