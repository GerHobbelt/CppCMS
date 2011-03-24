#pragma once
#include <apps/admin/master.h>

namespace data {
	namespace admin {
		struct categories;
	}
}

namespace apps {
	namespace admin {
		class categories :public master {
		public:
			categories(cppcms::service &s);

		private:
			void prepare();
			void load_form(data::admin::categories &c);
		};
	}
}

