#pragma once
#include <apps/admin/master.h>

namespace apps {
	namespace admin {
		class users : public master {
		public:
			users(cppcms::service &s);
			void prepare();
		};
	}
}
