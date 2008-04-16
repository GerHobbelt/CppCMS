#include "cache.h"
#include <iostream>
using namespace cache;
using namespace std;


int main()
{
	thread_cache c(10);
	c.insert("3","odd,three","three",36);
//	c.print_all();
	c.insert("1","odd","one",15);
//	c.print_all();
	c.insert("2","even","two",13);
//	c.print_all();
	c.insert("4","even","four",3);
//	c.print_all();
	c.insert("5","odd","five",3);
//	c.print_all();
	c.insert("6","even","six",6);

	int i,k;
	for(k=0;k<2;k++){
		for(i='1';i<='6';i++) {
			char buf[2]={i,0};
			string out;
//					cout<<(int)(c.fetch_string(buf,out));
//			cout<<":"<<out<<endl;
		}
//		cout<<"ERASE...\n";
		c.drop_secondary("three");
		c.drop_primary("1");
		c.drop_primary("2");
		c.drop_primary("10");
//		c.print_all();
	}
	string tmp;
	c.fetch_gzip("6",tmp);
	cout<<tmp;
	return 0;
}
