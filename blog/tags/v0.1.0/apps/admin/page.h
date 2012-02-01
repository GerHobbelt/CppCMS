#pragma once
#include <apps/admin/master.h>

namespace data {
	namespace admin {
		struct page;
	}
}

namespace apps {
	namespace admin {
		class page :public master {
		public:
			page(cppcms::service &s);

		private:
			void prepare(std::string id);
			void prepare();
			void prepare_shared(int id = 0);
		};
	}
}

