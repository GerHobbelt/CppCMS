#pragma once
#include <apps/admin/master.h>

namespace data {
	namespace admin {
		struct options;
	}
}

namespace apps {
	namespace admin {
		class options :public master {
		public:
			options(cppcms::service &s);

		private:
			void prepare();
			void load_form(data::admin::options &c);
		};
	}
}

