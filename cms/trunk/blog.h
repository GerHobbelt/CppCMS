//
// C++ Interface: blog
//
// Description: 
//
//
// Author: artik <artik@art-laptop>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <cppcms/worker_thread.h>

class Blog {
	Url_Parser url;
	void main_page();
public:
	virtual void init();
	virtual void main();
	Blog() : url(this) {};
};