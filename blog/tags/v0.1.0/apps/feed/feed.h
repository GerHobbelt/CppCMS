#pragma once
#include <cppcms/application.h>
#include <apps/basic_master.h>

namespace data {
	namespace feed {
		struct master;
	}
}

namespace cppdb {
	class session;
}

namespace apps {
	namespace feed {
		class feed_master : public basic_master {
		public:
			feed_master(cppcms::service &s);
		private:
			void posts();
			void cats(std::string id);
			void comments();
		};
	}
}
