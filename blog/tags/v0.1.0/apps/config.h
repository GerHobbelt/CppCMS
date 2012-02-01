#pragma once

#include <apps/basic_master.h>

namespace apps {
	class config : public basic_master {
	public:
		config(cppcms::service &srv);
	private:
		void configure();
		void version();
	};
}
