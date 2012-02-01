#ifndef MASTER_H
#define MASTER_H

#include <cppcms/application.h>

namespace data { class master; }
namespace dbixx { class session; }
namespace apps {

class wiki;

class master : public cppcms::application {
protected:
	wiki &wi;
	dbixx::session &sql;
	std::string &locale;
public:
	master(wiki &w);
	void ini(data::master &);
};

} // namespace wiki


#endif


