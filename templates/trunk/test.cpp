#include <iostream>
#include "content.h"
#include "renderer.h"

#include <boost/format.hpp>

using namespace std;
using namespace tmpl;

bool counter(content &c,int &i)
{
	if(i<5) {
		string tmp=(boost::format("%d") % i).str();
		c["val"]=tmp;
		i++;
		return true;
	}
	else {
		i=0;
		return false;
	}

}

int main()
{
	try{
		
		content c;
		template_data tmpl("test.opcode");

		renderer r(tmpl);

		string out;
		c["istrue"]=true;
		c["isfalse"]=false;
		c["myref"]=string("foo_ref");
		int i=0;

		content::vector_t &l1=c.vector("list_1",1);
		l1[0]["val"]=string("One item");

		c.list("list_empty");
		c.signal("count",boost::bind(counter,_1,i));


		r.render(c,"main",out);
		cout<<out<<endl;
	}
	catch(std::exception &e){
		std::cout<<e.what()<<'\n';
	}
	return 0;
}

