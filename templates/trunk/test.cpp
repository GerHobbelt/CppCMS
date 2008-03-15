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

void v(boost::any const &a,string &o)
{
	if(a.type()==typeid(int)) {
		char buf[32];
		snprintf(buf,32,"%d",boost::any_cast<int>(a));
		o+=buf;
	}
}

void t(renderer &r)
{
	r.add_converter(typeid(int),v);
}

int main()
{
	try{
		
		content c;
		template_data tmpl("test.tmpl");

		renderer r(tmpl);
		t(r);

		string out;
		c["title"]=(int)-15;
		c["proc"]=string("text");
		c["someval"]=false;
		int i=0;
#if 1
		c.signal("list",boost::bind(mycallback,_1,i));
#else
	#if 1 
		content::vector_t &l=c.vector("list",2);
	#else 
		content::list_t &l=c.list("list");
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

