#pragma once

#include <cppcms/serialization.h>
#include <cppcms/view.h>
#include <cppcms/xss.h>
#include <data/basic_master.h>
#include <booster/function.h>

namespace data {
namespace blog {

	struct sidebar_info: public cppcms::serializable {
		
		struct page : public cppcms::serializable {
			int id;
			std::string title;
			void serialize(cppcms::archive &a)
			{
				a & id & title;
			}
		};


		struct cat : public cppcms::serializable { 
			int id;
			std::string name; 
			void serialize(cppcms::archive &a)
			{
				a & id & name;
			}
		};

		std::vector<page> pages;
		std::vector<cat> cats;

		void serialize(cppcms::archive &a)
		{
			a & pages & cats;
		}

	};

	struct master: public data::basic_master {
		booster::function<void()> load_sidebar;
		sidebar_info sidebar;
	};
} // blog
} // master
