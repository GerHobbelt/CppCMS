#include <iostream>
#include "content.h"
#include "renderer.h"

#include <boost/format.hpp>

using namespace std;
using namespace tmpl;

int main()
{
	try{
		
		content c;
		template_data tmpl("test.tmpl");

		renderer r(tmpl);

		string out;
		//c["name"]=string("artik");
		//c["proc"]=string("text");
		c["somename"]=string("Noone");
		int i=0;

/*		content::vector_t &l=c.vector("list",0);
		l[0]["link"]=string("me");
		l[0]["title"]=string("otherme");
		l[1]["link"]=string("artik");
		l[1]["title"]=string("mastik");*/
		r.render(c,"main",out);
		cout<<out<<endl;
	}
	catch(std::exception &e){
		std::cout<<e.what()<<'\n';
	}
	return 0;
}

