#include "parser.h"
#include <boost/regex.hpp>
#include <set>

#include <iostream>

using namespace boost;

regex e_if_not_def("^\\s*if\\s+not\\s+def\\s+([a-zA-Z_][a-zA-Z_0-9]*)\\s*$");
regex e_eif_not_def("^\\s*elsif\\s+not\\s+def\\s+([a-zA-Z_][a-zA-Z_0-9]*)\\s*$");

regex e_if_def("^\\s*if\\s+def\\s+([a-zA-Z_][a-zA-Z_0-9]*)\\s*$");
regex e_if_not("^\\s*if\\s+not\\s+([a-zA-Z_][a-zA-Z_0-9]*)\\s*$");
regex e_eif_def("^\\s*elsif\\s+def\\s+([a-zA-Z_][a-zA-Z_0-9]*)\\s*$");
regex e_eif_not("^\\s*elsif\\s+not\\s+([a-zA-Z_][a-zA-Z_0-9]*)\\s*$");
regex e_while_not("^\\s*while\\s+not\\s+([a-zA-Z_][a-zA-Z_0-9]*)\\s*$");
regex e_util_not("^\\s*until\\s+not\\s+([a-zA-Z_][a-zA-Z_0-9]*)\\s*$");

regex e_if("^\\s*if\\s+([a-zA-Z_][a-zA-Z_0-9]*)\\s*$");
regex e_eif("^\\s*elsif\\s+([a-zA-Z_][a-zA-Z_0-9]*)\\s*$");

regex e_while("^\\s*while\\s+([a-zA-Z_][a-zA-Z_0-9]*)\\s*$");

regex e_util("^\\s*until\\s+([a-zA-Z_][a-zA-Z_0-9]*)\\s*$");

regex e_call("^\\s*call\\s+([a-zA-Z_][a-zA-Z_0-9]*)\\s*$");
regex e_inc("^\\s*inc\\s+([a-zA-Z_][a-zA-Z_0-9]*)\\s*$");

regex e_repeat("^\\s*repeat\\s*$");
regex e_end("^\\s*end\\s*$");
regex e_else("^\\s*else\\s*$");

regex e_var("^\\s*([a-zA-Z_][a-zA-Z_0-9]*)\\s*$");

struct e_matcher {
	regex *e;
	int val;
	int no_param;
};

e_matcher patterns[] = {
	{ &e_if_not_def , T_IFNDEF },
	{ &e_eif_not_def, T_ELIFNDEF },
	{ &e_if_def , T_IFDEF },
	{ &e_eif_def , T_ELIFDEF },
	{ &e_if_not , T_IFN },
	{ &e_eif_not , T_ELIFN },
	{ &e_while_not , T_WHILE_NOT },
	{ &e_util_not , T_UNTIL_N },
	{ &e_if , T_IF },
	{ &e_eif, T_ELIF },
	{ &e_while , T_WHILE },
	{ &e_util, T_UNTIL },
	{ &e_call , T_CALL },
	{ &e_inc, T_INCLUDE },
	{ &e_repeat , T_REPEAT ,1 },
	{ &e_else , T_ELSE ,1 },
	{ &e_end, T_END ,1},
	{ &e_var, T_VAR } };
	
	


Parser::Parser(FILE *fin)
{
	f=fin;
	line=1;
}

int Parser::getc()
{
	int c;
	if(unget_stack.empty()) {
		return fgetc(f);
	}
	else {
		c=unget_stack.top();
		unget_stack.pop();
	}
	return c;
}

void Parser::ungetc(int c)
{
	unget_stack.push(c);
}

void Parser::error(char const *s)
{
	char text[32];
	snprintf(text,32," at line %d",line);
	throw string(s)+text;
}

int Parser::get_tocken(string &s)
{
	int c,c1,c2;
	s="";

	c1=getc();
	c2=getc();
	
	if(c1=='<' && c2=='%') {
		while((c=getc())!=EOF) {
			if(c=='%'){
				if((c2=getc())=='>') {
					return T_SOME_CODE;
				}
				else {
					ungetc(c2);
				}
			}
			s+=(char)c;
			if(c=='\n') line++;
		}
		error("Expecting `%>' found EOF");
	}
	ungetc(c2);
	ungetc(c1);

	while((c=getc())!=EOF) {
		if(c=='<') {
			if((c2=getc())=='%') {
				ungetc(c2);
				ungetc(c);
				return T_INLINE;
			}
			else {
				ungetc(c2);
			}
		}
		s+=(char)c;
		if(c=='\n') line++;
	}
	if(s.size()==0) {
		return T_EOF;
	}
	return T_INLINE;
}

int Templates_Parser::get_tocken(string &s)
{
	int tocken=Parser::get_tocken(s);
	if(tocken==T_INLINE || tocken==T_EOF) {
		return tocken;
	}
	else {
		unsigned i=0;
		for(i=0;i<sizeof(patterns)/sizeof(patterns[0]);i++) {
			cmatch match;
			if(regex_match(s.c_str(),match,*(patterns[i].e))) {
				if(patterns[i].no_param) {
					s="";
				}
				else {
					s=match[1];
				}
				return patterns[i].val;
			}
		}
		error("Syntax error");
	}
	return 0;
}




map<string,int> variables;
map<string,int> templates;

int var(string &s)
{
	map<string,int>::iterator i;
	if((i=variables.find(s))!=variables.end()){
		return i->second;
	}
	else {
		throw "Undeclared variable: " + s;
	}
}

int templ(string &s)
{
	map<string,int>::iterator i;
	if((i=templates.find(s))!=templates.end()){
		return i->second;
	}
	else {
		throw "Undeclared template: " + s;
	}
}


bool load_vars(char const *name,char *h_file)
{
	set<string> vars;
	set<string> templs;
	regex comment("^(\\s*|\\s*#.*)$");
	regex vars_d("^\\s*variables\\s*:\\s*$");
	regex tpls_d("^\\s*templates\\s*:\\s*$");
	regex var_t("^\\s*([a-zA-Z_][a-zA-Z_0-9]*)\\s*$");
	
	FILE *f=fopen(name,"r");
	if(!f) {
		cerr<<"Failed to open file:"<<name<<endl;
		return false;
	}
	char buffer[81];
	int state=0;
	int line=0;
	set<string>::iterator i;
	int n=0;
	while(fgets(buffer,81,f)) {
		line++;
		int len=strlen(buffer);

		if(len==80) {
			cerr<<"Lines should not exceed 80 chars len in "
				<<name<<" line:"<<line <<endl;
			goto err;
		}
		cmatch match;
		if(regex_match(buffer,match,comment)) {
			continue;
		}
		switch(state){
		case 0:
			if(!regex_match(buffer,match,tpls_d)){
				cerr<<"`templates:' expected at line "<<line<<endl;	
				goto err;				
			}
			state=1;
			break;
		case 1:
			if(regex_match(buffer,match,vars_d)){
				state=2;
			}
			else if(regex_match(buffer,match,var_t)) {
				templs.insert(match[1]);
			}
			else {
				cerr<<"template name expected at line "<<line<<endl;
				goto err;
			}
			break;
		case 2:
			if(regex_match(buffer,match,var_t)){
				vars.insert(match[1]);
			}
			else {
				cerr<<"variable name expected at line "<<line<<endl;
				goto err;
			}
		}
	}
	if(state!=2) {
		cerr<<"Unexpected EOF in "<<name<<endl;
		goto err;
	}
	fclose(f);
	if(h_file) {
		f=fopen(h_file,"w");
		if(!f){
			cerr<<"Failed to open file "<<h_file<<endl;
			return false;
		}
		fprintf(f,"#ifndef _TEMPLATE_CODES_H_\n");
		fprintf(f,"#defile _TEMPLATE_CODES_H_\n\n");
	}
	
	for(i=vars.begin();i!=vars.end();i++) {
		if(h_file) {
			fprintf(f,"#define TMPLV_%s %d\n",i->c_str(),n);
		}
		variables[*i]=n;
		n++;
	}
	if(h_file) fprintf(f,"\n#define TMPL_VAR_NUM %d\n\n",n);

	n=0;
	for(i=templs.begin();i!=templs.end();i++) {
		if(h_file) {
			fprintf(f,"#define TMPL_%s %d\n",i->c_str(),n);
		}
		templates[*i]=n;
		n++;
	}
	if(h_file) {
		fprintf(f,"\n#define TMPL_TEMPL_NUM %d\n",n);
		fprintf(f,"\n#endif\n");
	}
	fclose(f);
	return true;
err:
	fclose(f);
	return false;
}
