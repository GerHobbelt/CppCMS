#include <cppcms/application.h>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include "data.h"

using namespace std;
using namespace cppcms;

class hello: public application {
public:
    hello(worker_thread &worker) :
        application(worker) 
    {
	    url.add("^/?$",
	    	boost::bind(&hello::info,this));
	    use_template("view");
    }
    void info()
    {
	    data::message c;
	    c.arg=0;
	    c.fact=1;
	    if(env->getRequestMethod()=="POST") {
		    c.info.load(*cgi);
		    if(c.info.validate()) {
		        c.arg=c.info.arg.get(); 
			c.info.clear();
		    }
		    else { // No cache should be used
		   	render("message",c);
			return;
		    }
	    }

	    string key="factorial_"+boost::lexical_cast<string>(c.arg);
	    if(cache.fetch_page(key))
	    	return;
	    long long int f=1;
	    for(int i=1;i<=c.arg;i++) {
	    	f*=i;
	    }
	    c.fact=f;
	    render("message",c);
	    cache.store_page(key,3600);
    }
};

int main(int argc,char ** argv)
{
    try {
        manager app(argc,argv);
        app.set_worker(new application_factory<hello>());
        app.execute();
    }
    catch(std::exception const &e) {
        cerr<<e.what()<<endl;
    }
}
