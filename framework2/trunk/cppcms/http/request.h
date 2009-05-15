#ifndef CPPCMS_HTTP_REQUEST_H
#define CPPCMS_HTTP_REQUEST_H
#include <cppcms/defs.h>
#include <cppcms/util/noncopyable.h>
#include <string>
#include <map>
#include <vector>

namespace cppcms {

namespace http {

	class cookie;
	class file;

	class request_impl;
	class CPPCMS_API request : public util::noncopyable {
	public:
		// get env like
		bool https();

		std::string server_software();
		std::string server_name();
		std::string gateway_interface();

		std::string server_protocol();
		unsigned server_port();
		std::string request_method();
		std::string path_info();
		std::string path_translated();
		std::string script_name();
		std::string script_filename();
		std::string server_admin();
		std::string server_signature();
		std::string query_string();
		std::string remote_host();
		std::string remote_addr();
		std::string auth_type();
		std::string remote_user();
		std::string remote_ident();
		std::string request_uri();
		std::string content_type();
		unsigned long long content_length();
		std::string http_accept();
		std::string http_user_agent();
		std::string http_accept_encoding();
		std::string accept_language();
		std::string document_root();
		std::string http_cookie();
		std::string http_forwarded();
		std::string http_host();
		std::string http_forwarder();
		std::string http_referrer();

		// Other
		std::string getenv(std::string const &);
		
		typedef std::multimap<std::string,std::string> form_type;
		typedef std::map<std::string,cookie> cookies_type;
		typedef std::vector<file> files_type;

		cookies_type const &cookies();
		form_type const &get();
		form_type const &post();
		files_type const &files();

	private:
		request(request const &); // Non copyable
		request const &operator=(request const &); // Non assignable
	public:
		request(connection &);
		bool prepare();
		~request();
	private:

		form_type get_;
		form_type post_;
		files_type files_;
		cookies_type cookies_;
		connection *connection_;
		std::auto_ptr<request_impl> p;
	};


} // namespace http

} // namespace cppcms



#endif
