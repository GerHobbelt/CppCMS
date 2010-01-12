#include <cppcms/application.h>
#include <cppcms/manager.h>

#include "data.h"

using namespace std;
using namespace cppcms;

class my_hello_world : public application {
public:
    my_hello_world(worker_thread &s) :
       application(s)
    {
    	use_template("my_view");
    };
    virtual void main();
};

void my_hello_world::main()
{
    data::message c;
    c.message=">>>Hello<<<";
    render("message",c);
}

int main(int argc,char ** argv)
{
    try {
        manager app(argc,argv);
        app.set_worker(new application_factory<my_hello_world>());
        app.execute();
    }
    catch(std::exception const &e) {
        cerr<<e.what()<<endl;
    }
}

