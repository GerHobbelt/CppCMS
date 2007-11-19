#include <iostream>
#include <stdio.h>

using namespace std;

#include "parser.h"

void compile(string defname,string fname,FILE *fin,FILE *fout)
{
	Parser parser(fin);
	int i;
	string s;
	string header=filename_to_define(fname);
	
	fprintf(fout,"#ifndef %s\n#define %s\n\n",header.c_str(),header.c_str());
	int tok;
	fprintf(fout,"#define %s \\\n",defname.c_str());
	while((tok=parser.get_tocken(s))!=T_EOF) {
		if(tok==T_INLINE) {
			fprintf(fout," TEMPLATE_OUTPUT(\"");
			for(i=0;i<(int)s.size();i++) {
				int c=s[i];
				if(0<c && c<32) {
					fprintf(fout,"\\x%02x",c);
				}
				else {
					fputc(c,fout);
				}
				
			}
			fprintf(fout,"\");\\\n");
		}
		else if(tok==T_SOME_CODE) {
			for(i=0;i<(int)s.size();i++) {
				int c=s[i];
				if(c=='\n') {
					fprintf(fout,"\\\n");
				}
				else {
					fputc(c,fout);
				}
			}
		}
	}
	
	fprintf(fout,"\n\n#endif\n");
}


int main(int argc,char *argv[])
{
	if(argc!=4) {
		cerr<<"Usage: compile define-name file-in.tmpl file-out.h\n";
		return 1;
	}
	string def_name=argv[1];
	FILE *fin=NULL,*fout=NULL;
	if(!(fin=fopen(argv[2],"r")) || !(fout=fopen(argv[3],"w"))){
		cerr<<"Failed to open files";
		return 1;
	}
	try {
		compile(def_name,argv[2],fin,fout);
	}
	catch(string &s)
	{
		cerr<<s<<endl;
		return 1;
	}
	
	fclose(fin);
	fclose(fout);
	
	return 0;
}
