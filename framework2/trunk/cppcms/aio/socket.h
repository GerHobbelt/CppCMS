#ifndef CPPCMS_AIO_SOCKET_H
#define CPPCMS_AIO_SOCKET_H

#include <cppcms/util/noncopyable.h>

namespace cppcms  {
	namespace aio {
		
		struct tcp {};
		struct unix {};
		struct udp {};
	
		class endpoint;

		class socket : public util::noncopyable {
		public:
			socket();
			~socket();

			void async_connect(endpoint const &,error_handler);
			void async_read_some(void *buffer,size_t s,io_handler);
			void async_write_some(void *buffer,size_t s,io_handler);
			void close();
		private:
			std::auto_ptr<socket_impl> p;
		};


	} // aio
} // cppcms

#endif
