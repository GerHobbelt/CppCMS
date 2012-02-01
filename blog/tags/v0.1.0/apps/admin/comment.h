#pragma once
#include <apps/admin/master.h>

namespace data {
	namespace admin {
		struct comment;
	}
}

namespace apps {
	namespace admin {
		class comment :public master {
		public:
			comment(cppcms::service &s);

		private:
			void prepare(std::string id);
		};
	}
}

