#include <cppcms/worker_thread.h>
#include <cppcms/manager.h>
#include <tmpl/content.h>
#include <tmpl/renderer.h>
#include <iostream>
using namespace std;
using namespace cppcms;
using namespace tmpl;

class my_hello_world : public worker_thread {
    renderer rnd;
public:
    my_hello_world(manager const &s,template_data const &t) :
        worker_thread(s),
	rnd(t)
    {
    };
    virtual void main();
};

void my_hello_world::main()
{
    content c;
    c["message"]=string("Hello");
    rnd.render(c,"main",out);
}

int main(int argc,char ** argv)
{
    template_data templ;
    try {
        manager app(argc,argv);
        templ.load(app.config.sval("templates.file"));
	app.set_worker(
	    new one_param_factory<my_hello_world,template_data>(templ));
	app.execute();
    }
    catch(std::exception const &e) {
        cerr<<e.what()<<endl;
    }
}

