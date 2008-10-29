#ifndef MASTER_H
#define MASTER_H

#include <cppcms/application.h>

class wiki;
namespace data { class master; }

namespace dbixx { class session; }

namespace apps {

class master : public cppcms::application {
protected:
	wiki &wi;
	dbixx::session &sql;
	std::string &locale;
public:
	master(wiki &w);
	void ini(data::master &);
};

}


#endif


