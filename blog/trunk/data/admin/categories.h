#pragma once
#include <data/admin/master.h>
#include <booster/shared_ptr.h>
#include <vector>

namespace data {
	namespace admin {
		struct categories : public master {
			struct entry {
				std::string name;
				int id;
			};
			std::string error_message;
			std::vector<entry> current_categories;
		};
	} // admin
} // data

