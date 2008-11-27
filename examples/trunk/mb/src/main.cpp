#include "mb.h"

using namespace std;
using namespace cppcms;

int main(int argc,char ** argv)
{
    try {
        manager app(argc,argv);
        app.set_worker(new application_factory<apps::mb>());
        app.execute();
    }
    catch(std::exception const &e) {
        cerr<<e.what()<<endl;
    }
}

