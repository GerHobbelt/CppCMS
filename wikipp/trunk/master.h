#ifndef MASTER_H
#define MASTER_H

#include <string>

class wiki;
namespace data {
	class master;
}

namespace apps {

class master {
protected:
	wiki &wi;
	std::string &locale;
public:
	master(wiki &w);
	void ini(data::master &);
};

}


#endif


