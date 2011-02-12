#pragma once

#include <apps/admin/master.h>

namespace apps {
	namespace admin {
		class caching : public master {
		public:
			caching(cppcms::service &s);
		private:
			void prepare();
		};
	}
}
