#include <cppcms/application.h>
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
	    	boost::bind(&hello::say_hello,this));
	    use_template("view");
    }
    void say_hello()
    {
	    data::message c;
	    if(env->getRequestMethod()=="POST") {
		    c.info.load(*cgi);
		    if(c.info.validate()) {
			    c.name=c.info.name.get();
			    c.sex=c.info.sex.get();
			    c.state=c.info.matrial.get();
			    c.age=c.info.age.get();
		    	    c.info.clear();
		    }
	    }
	    render("message",c);
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
