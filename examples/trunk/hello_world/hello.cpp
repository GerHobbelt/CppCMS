#include <cppcms/application.h>
#include <iostream>
using namespace std;
using namespace cppcms;

class my_hello_world : public application {
public:
    my_hello_world(worker_thread &worker) :
        application(worker) 
    {
    };
    virtual void main();
};

void my_hello_world::main()
{
    cout<<"<html>\n"
          "<body>\n"
          "  <h1>Hello World</h1>\n"
          "</body>\n"
          "</html>\n";
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
