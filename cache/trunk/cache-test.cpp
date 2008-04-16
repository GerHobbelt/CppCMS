#include "cache.h"
#include <iostream>
using namespace cache;
using namespace std;


int main()
{
	thread_cache c(5);
	c.insert("1","odd","one",10);
	c.insert("2","even","two",10);
	c.insert("3","odd","three",10);
	c.insert("4","even","four",10);
	c.insert("5","odd","five",10);
	c.insert("6","even","six",10);

	int i,k;
	for(k=0;k<2;k++){
		for(i='1';i<='6';i++) {
			char buf[2]={i,0};
			string out;
			cout<<c.fetch_string(buf,out)<<":"<<out<<endl;
		}
		c.drop_secondary("odd");
	}
	return 0;
}
