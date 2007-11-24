#ifndef _PARSER_H
#define _PARSER_H

#include <stdio.h>

#include <stack>
#include <map>
#include <string>

using namespace std;

enum {
	T_EOF,
	T_INLINE,
	T_CALL,
	T_VAR,
	T_INCLUDE,
	T_INCLUDE_REF,
	T_IF, T_IFN, T_IFDEF, T_IFNDEF,
	T_ELIF, T_ELIFN, T_ELIFDEF, T_ELIFNDEF,
	T_WHILE, T_WHILE_NOT,
	T_REPEAT,T_UNTIL, T_UNTIL_N,
	T_ELSE,
	T_END,
	T_SOME_CODE
};

string filename_to_define(string fname);

class Parser{
	int line;
	FILE *f;
	int getc();
	void ungetc(int);
	stack<int> unget_stack;
protected:
	void error(char const *);
public:
	Parser(FILE *fin);
	int get_tocken(string &s);
};

class Templates_Parser : public Parser {
public:
	Templates_Parser(FILE *fin) : Parser(fin) { };
	int get_tocken(string &s);
};

extern map<string,int> variables;
extern map<string,int> templates;

int var(string &s);
int templ(string &s);

bool load_vars(char const *name,char *h_file=NULL,char const *prefix=NULL);

#endif /* _PARSER_H */
