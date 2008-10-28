#ifndef MASTER_H
#define MASTER_H

class wiki;
namespace data {
	class master;
}

class master {
protected:
	wiki &wi;
public:
	master(wiki &w);
	void ini(data::master &);
};

#endif


