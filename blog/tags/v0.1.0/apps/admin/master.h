#pragma once
#include <apps/basic_master.h>

namespace apps {
	namespace admin {
		class master : public basic_master {
		public:
			master(cppcms::service &s) : basic_master(s) {}
		};
	}
};
