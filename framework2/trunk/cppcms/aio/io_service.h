#ifndef CPPCMS_AIO_IO_SERVICE_H
#define CPPCMS_AIO_IO_SERVICE_H

#include <cppcms/util/noncopyable.h>
#include <memory>

namespace cppcms {
	namespace aio {
		class io_service : public util::noncopyable {
		public:
			void run();
			void post(util::handler h);
			io_service();
			~io_service();
		private:
			std::auto_ptr<io_service_impl> p;
		};

	} // namespace aio
} // namespace cppcms




#endif
