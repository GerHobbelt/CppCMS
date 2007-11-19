#include <iostream>
#include <stdio.h>
#include <boost/scoped_array.hpp>
#include <boost/regex.hpp>


#include "parser.h"


using namespace std;
using namespace boost;

int link_all(string &base_dir, string &out)
{
	map<string,int>::iterator i_it;
	int n;
	uint32_t size;
	FILE *f;
	if(templates.rbegin()!=templates.rend()) {
		size=templates.rbegin()->second+1;
	}
	else {
		cerr<<"No templates to procceed\n";
		return 1;
	}
	
	if((f=fopen(out.c_str(),"wb"))==NULL) { // wb - for windows
		cerr<<"Failed to open file:"<<out<<endl;
		return 1;
	}
	
	scoped_array<uint32_t> sizes(new uint32_t [size]);
	
	memset(sizes.get(),0,size*4);
	fwrite(&size,1,4,f);
	fwrite(sizes.get(),4,size,f);

	for(i_it=templates.begin(),n=0;i_it!=templates.end();i_it++,n++){
		if(i_it->second!=n) {
			cerr<<"Internal error\n";
			return 1;
		}
		string fname;
		fname=base_dir+i_it->first+".op";
		FILE *fin=fopen(fname.c_str(),"rb");
		if(!fin) {
			cerr<<"Failed to open file:"<<fname<<endl;
			return 1;
		}
		char buffer[256];
		int r;
		uint32_t counter=0;
		while((r=fread(buffer,1,256,fin))>0) {
			fwrite(buffer,1,r,f);
			counter+=r;
		}
		fclose(fin);
		sizes[n]=counter;
	}
	fseek(f,4,SEEK_SET);
	fwrite(sizes.get(),4,size,f);
	fclose(f);
	return 0;
}

void help()
{
	cerr<<"Usage: interface.def [-d directory ] [-o output]\n";
}

int main(int argc,char *argv[])
{
	string base_dir="";
	char *interface=NULL;
	string output="";
	int i;
	
	for(i=1;i<argc;i++){
		if(strcmp(argv[i],"-d")==0 && i+1<argc && base_dir=="") {
			i++;
			base_dir=argv[i];
		}
		else if(strcmp(argv[i],"-o")==0 && i+1<argc && output=="") {
			i++;
			output=argv[i];
		}
		else if(interface==NULL) {
			interface=argv[i];
		}
		else {
			help();
			return 1;
		}
	}
	if(interface==NULL){
		help();
		return 1;
	}
	if(output=="") {
		regex rename("^(.*)\\.def$");
		cmatch m;
		if(regex_match(interface,m,rename)){
			output=m[1]+".template";
		}
		else {
			help();
			return 1;
		}
	}
	
	if(!load_vars(interface,NULL,NULL)) {
		return 1;
	}
	
	return link_all(base_dir,output);
}
