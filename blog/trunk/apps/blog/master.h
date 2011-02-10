#pragma once
#include <cppcms/application.h>
#include <cppcms/base_content.h>
#include <apps/basic_master.h>

namespace cppdb {
	class session;
}

namespace data { 
	namespace blog {
		struct master;
	}
}


namespace apps {

	namespace blog {

		class master : public basic_master {
		public:
			master(cppcms::service &s);
			~master();
			
			virtual void init();
			virtual void clear();
			void load_sidebar(data::blog::master &c);
		protected:
			void prepare(data::blog::master &c);
			std::string user_;
		};
	} // blog
} // apps
