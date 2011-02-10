#pragma once
#include <cppcms/application.h>

namespace cppdb {
	class session;
}
namespace data {
	struct basic_master;
}
namespace apps {
	class basic_master : public cppcms::application {
	public:
		basic_master(cppcms::service &s);
		~basic_master();
		
		virtual void clear();
	protected:
		std::string host_;
		cppdb::session &sql();
		void prepare(data::basic_master &);
	private:
		std::auto_ptr<cppdb::session> sql_;
		std::string conn_str_;
		std::string media_;
		std::string cookie_;
	};
}

