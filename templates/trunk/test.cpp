#include <iostream>
#include "content.h"
#include "renderer.h"

#include <boost/format.hpp>

using namespace std;
using namespace tmpl;

bool mycallback(content &c,int &i)
{
	if(i<=10){
		c["author"]=(boost::format("%d") % i ).str();
		c["content"]=(boost::format("%d") % (i*i) ).str();
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
		template_data tmpl("test.tmpl");

		renderer r(tmpl);

		string out;
		c["title"]=string("title\n");
		c["proc"]=string("text");
		c["someval"]=false;
		int i=0;
#if 1
		c.signal("list",boost::bind(mycallback,_1,i));
#else
#if 0		
		content::vector_t &l=c.vector("list",2);
#elsif
		content::list_t &l=c.vector("list");
#endif

		l.push_back(content());
		l.back()["author"]=string("me");
		l.back()["content"]=string("otherme");

		l.push_back(content());
		l.back()["author"]=string("artik");
		l.back()["content"]=string("mastik");
#endif
		
		r.render(c,"main",out);
		cout<<out<<endl;
	}
	catch(std::exception &e){
		std::cout<<e.what()<<'\n';
	}
	return 0;
}

