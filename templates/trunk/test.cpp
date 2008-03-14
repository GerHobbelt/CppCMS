#include <iostream>
#include "content.h"
#include "renderer.h"

using namespace std;
using namespace tmpl;


int main()
{
	try{
		
		content c;
		template_data tmpl("test.tmpl");
		
		renderer r(tmpl);
		string out;
		c["title"]=string("title\n");
		
		content::vector_t &l=c.vector("list",2);

		l.push_back(content());
		l.back()["author"]=string("me");
		l.back()["content"]=string("otherme");

		l.push_back(content());
		l.back()["author"]=string("artik");
		l.back()["content"]=string("mastik");
		
		r.render(c,"main",out);

		cout<<out<<endl;
	}
	catch(std::exception &e){
		std::cout<<e.what()<<'\n';
	}
	return 0;
}

