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

		struct link_cat : public cppcms::serializable  {
			struct link : public cppcms::serializable {
				std::string href;
				std::string title;
				std::string description;
				void serialize(cppcms::archive &a)
				{
					a & href & title & description;
				}
			};
			
			std::string title;
			std::list<link> links;

			void serialize(cppcms::archive &a)
			{
				a & title & links ;
			}
		};

		std::vector<page> pages;
		std::vector<cat> cats;
		std::list<link_cat> link_cats;

		void serialize(cppcms::archive &a)
		{
			a & pages & cats & link_cats; 
		}

	};

	struct master: public data::basic_master {
		booster::function<void()> load_sidebar;
		sidebar_info sidebar;
	};
} // blog
} // master
