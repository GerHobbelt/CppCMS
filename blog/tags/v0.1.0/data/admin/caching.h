#pragma once
#include <data/admin/master.h>
namespace data {
	namespace admin {
		struct caching : public master {
			size_t size;
			bool is_active;
			caching() : size(0),is_active(false){}
		};
	}
}
