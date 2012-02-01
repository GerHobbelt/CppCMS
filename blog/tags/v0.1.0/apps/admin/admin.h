#pragma once 
#include <cppcms/application.h>
namespace apps {
	namespace admin {
		class admin_master : public cppcms::application {
		public:
			admin_master(cppcms::service &srv);
			void main(std::string);
		};
	}
}
