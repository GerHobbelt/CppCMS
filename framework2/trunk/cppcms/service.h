#ifndef CPPCMS_SERVICE_H
#define CPPCMS_SERVICE_H

#include <cppcms/defs.h>
#include <cppcms/util/noncopyable.h>

namespace cppcms {

	class CPPCMS_API service : public util::noncopyable {
	public:
		void mount(std::string path_prefix,std::auto_ptr<application_factory> factory);
		service(int argv,char **argc);
		void run();
	};

} // cppcms


#endif
