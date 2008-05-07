#include <iostream>
#include "content.h"
#include "renderer.h"

#include <boost/format.hpp>
#include <boost/bind.hpp>

using namespace std;
using namespace tmpl;

struct comp { double x_,y_; comp(double x=0,double y=0) : x_(x), y_(y) {} ;};

void my_conv(boost::any const &val,std::string &out)
{
	comp my=boost::any_cast<comp>(val);
	char buf[64];
	snprintf(buf,sizeof(buf),"< %f + %f i>",my.x_,my.y_);
	out.append(buf);
}

void ToUpper(string in,string &out,char const *p)
{
	int i;
	if(p)
		out.append(p);
	for(i=0;i<in.size();i++){
		char c=in[i];
		if('a'<= c && c<='z')
			c+='A'-'a';
		out+=c;
	}
}

void TypeName(boost::any const &a,string &out,char const *p)
{
	if(p)
		out.append(p);
	out.append(a.type().name());
}


int main()
{
	try{
		
		content c;
		template_data tmpl("test.look");

		renderer r(tmpl);

		string out;
		
		c["str"]=string("<test>");
		std::tm t;
		time_t t2;
		t2=time(NULL);
		localtime_r(&t2,&t);
		c["d"]=t;
		c["i"]=32767;
		c["c"]=comp(1,0);
		c["n1"]=1;
		c["n2"]=2;
		c["n3"]=3;
		c["n11"]=11;
		c["v1"]=string("var one");
		c["v2"]=string("var two");
		
		r.add_converter( typeid(comp),boost::bind(&my_conv,_1,_2));
		r.add_string_filter("toupper",boost::bind(ToUpper,_1,_2,_3));
		r.add_any_filter("typename",boost::bind(TypeName,_1,_2,_3));
		//c.signal("count",boost::bind(counter,_1,i));

	//	transtext::trans_gnu tr;
		transtext::trans_thread_safe tr;

		tr.load("he","test","./transtext/locale");
		r.set_translator(tr);
		r.render(c,"main",out);
		cout<<out<<endl;
	}
	catch(std::exception &e){
		std::cout<<e.what()<<'\n';
	}
	return 0;
}

