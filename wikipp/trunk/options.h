#ifndef OPTIONS_H
#define OPTIONS_H
#include <cppcms/archive.h>
#include "data.h"
#include "master.h"

class wiki;

namespace apps {

using namespace cppcms;

struct global_options : public serializable {
	int users_only_edit;
	virtual void load(archive &a);
	virtual void save(archive &a) const;
};

struct locale_options : public serializable {
	string title;
	string about;
	string copyright;
	virtual void load(archive &a);
	virtual void save(archive &a) const;
};

class options : public master {
	bool loaded;
protected:
	void edit();
	void reset();
public:
	options(wiki &);
	void load();
	void save();
	string edit_url();
	global_options global;
	locale_options  local;
};

} // apps;

#endif
