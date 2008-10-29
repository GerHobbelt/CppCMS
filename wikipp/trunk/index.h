#ifndef INDEX_H
#define INDEX_H

#include "data.h"
#include "master.h"

namespace apps {

class index : public master {
public:
	index(wiki &);
	void display_index();
	std::string index_url();
};

}


#endif
