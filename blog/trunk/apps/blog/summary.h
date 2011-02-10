#pragma once
#include <apps/blog/post.h>

namespace data {
	namespace blog {
		struct summary;
	}
}

namespace apps {
	namespace blog {
		class summary : public master {
		public:
			summary(cppcms::service &s);
			void category(std::string cat_id);
			void category(std::string cat_id,std::string from_page);
			void all();
			void all(std::string from_page);
		private:
			bool prepare(int cat_id,int page);
		};
	}
}
