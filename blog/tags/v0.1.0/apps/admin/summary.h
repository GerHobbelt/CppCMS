#pragma once

#include <apps/admin/master.h>

namespace apps {
	namespace admin  {
		class summary  : public master {
		public:
			summary(cppcms::service &s);
			
			void prepare();			
		};
	}
}
