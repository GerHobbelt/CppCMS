#pragma once
#include <apps/admin/master.h>

namespace data {
	namespace admin {
		struct post;
	}
}

namespace apps {
	namespace admin {
		class post :public master {
		public:
			post(cppcms::service &s);

		private:
			void prepare(std::string id);
			void prepare();
			void prepare_shared(int id = 0);
		};
	}
}

