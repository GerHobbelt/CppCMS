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
	    url.add("^/(en|he)/?$",
	    	boost::bind(&hello::say_hello,this,_1));
	    use_template("view");
    }
    void say_hello(string lang)
    {
	    set_lang(lang);
	    data::message c;
	    c.message=gettext("Hello World");
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
