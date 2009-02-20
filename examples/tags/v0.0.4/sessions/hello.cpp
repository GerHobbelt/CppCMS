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
	    	boost::bind(&hello::info,this));
	    use_template("view");
    }
    void info()
    {
	    data::message c;
	    if(env->getRequestMethod()=="POST") {
		    c.info.load(*cgi);
		    if(c.info.validate()) {
			    session["name"]=c.info.name.get();
			    session["sex"]=c.info.sex.get();
			    session["state"]=c.info.martial.get();
			    session.set("age",c.info.age.get());
			    c.info.clear();
		    }
	    }

	    if(session.is_set("name")) {
		c.name=session["name"];
		if(session["sex"]=="Male") {
			c.who="Mr";
		}
		else {
			if(session["state"]=="Single") {
				c.who="Miss";
			}
			else {
				c.who="Mrs";
			}
		}
		c.age=session.get<double>("age");
	    }
	    else {
		c.name="Visitor";
		c.age=-1;
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
