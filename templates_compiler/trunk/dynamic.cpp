#include <iostream>
#include <stdio.h>

#include "parser.h"
#include "bytecode.h"

using namespace std;


// Map file format:
// file = file line | ;
// line = prefix OFFSET NAME;
// prefix = T | N

int exit_status=0;

struct stack_info{
	int operation;
	int position;
// Functions
	stack_info(int a,int b) { operation=a; position=b; };
	stack_info() {};
};

stack<stack_info> call_stack;



bool check_names(char *name,string &fopcode)
{
	static const char ending[]=".tmpl";
	int len=strlen(name);
	if(len<6 || strcmp(name+len-5,ending)!=0) {
		cerr<<"Incorrect input file name: "<<name<<endl;
		cerr<<"Must be filename.tmpl";
		exit_status=1;
		return false;
	}
	fopcode="";
	fopcode.append(name,len-5);
	fopcode+=".op";
	return true;
}


#define OP_LEN  (sizeof(Tmpl_Op))
#define OP_OFF  (offsetof(Tmpl_Op,parameter))
#define OP_OFFJMP  (offsetof(Tmpl_Op,jump))

#define WRITE_OP() fwrite(&op,1,OP_LEN,fop); pos+=OP_LEN;

#define SET_JUMP(ptr)					\
{										\
	uint32_t tmp=pos;					\
	fseek(fop,ptr+OP_OFFJMP,SEEK_SET);	\
	fwrite(&tmp,1,4,fop);				\
	fseek(fop,pos,SEEK_SET);			\
}

	
enum {
	TOP_IF, TOP_LOOP, TOP_REPEAT, TOP_ELSE
};

void compile(char *fin_name,string fop_name)
{
	FILE *fin=NULL,*fop=NULL;
	try{
		if((fin=fopen(fin_name,"r"))==NULL)
			throw string("Failed to open ")+fin_name;
		if((fop=fopen(fop_name.c_str(),"wb"))==NULL)
			throw string("Failed to open ")+fop_name;
		
		while(!call_stack.empty()) call_stack.pop();

		Templates_Parser parser(fin);		
		
		int pos=0;
		int tmp_pos;
		string s;
		Tmpl_Op op;
		bool finished=false;
		
		while(!finished) {
			s="";
			int t=parser.get_tocken(s);
			memset(&op,OP_LEN,0);
			switch(t){
				case T_INLINE:
					op.opcode=OP_INLINE;
					op.parameter=s.size();
					op.jump=pos+OP_LEN+s.size();
					WRITE_OP();
					fwrite(s.c_str(),1,s.size(),fop);
					pos=op.jump;
					break;
				case T_CALL:
					op.opcode=OP_CALL;
					op.parameter=var(s);
					WRITE_OP();
					break;
				case T_VAR:
					op.opcode=OP_VAR;
					op.parameter=var(s);
					WRITE_OP();
					break;
				case T_INCLUDE:
					op.opcode=OP_INCLUDE;
					op.parameter=templ(s);
					WRITE_OP();
					break;
				case T_IF:
				case T_IFDEF:
				case T_IFN:
				case T_IFNDEF:
					if(t==T_IF) op.opcode=OP_GOTO_IF_FALSE;
					else if(t==T_IFN) op.opcode=OP_GOTO_IF_TRUE;
					else if(t==T_IFDEF) op.opcode=OP_GOTO_IF_NDEF;
					else if(t==T_IFNDEF) op.opcode=OP_GOTO_IF_DEF;
					call_stack.push(stack_info(TOP_IF,pos));
					op.parameter=var(s);
					WRITE_OP();
					break;
				case T_ELIF:
				case T_ELIFDEF:
				case T_ELIFN:
				case T_ELIFNDEF:
					if(call_stack.empty() || call_stack.top().operation!=TOP_IF)
						throw string("ELSEIF without IF");
					SET_JUMP(call_stack.top().position);
					call_stack.top().position=pos;
					if(t==T_ELIF) op.opcode=OP_GOTO_IF_FALSE;
					else if(t==T_ELIFN) op.opcode=OP_GOTO_IF_TRUE;
					else if(t==T_ELIFDEF) op.opcode=OP_GOTO_IF_NDEF;
					else if(t==T_ELIFNDEF) op.opcode=OP_GOTO_IF_DEF;
					op.parameter=var(s);
					WRITE_OP();
					break;
				case T_ELSE:
					if(call_stack.empty() || call_stack.top().operation!=TOP_IF)
						throw string("ELSE without IF");
					tmp_pos=call_stack.top().position;
					call_stack.top().operation=TOP_ELSE;
					call_stack.top().position=pos;
					op.opcode=OP_GOTO;
					op.jump=0;
					WRITE_OP();
					SET_JUMP(tmp_pos);
					break;
				case T_WHILE:
				case T_WHILE_NOT:
					if(t==T_WHILE) op.opcode=OP_GOTO_IF_FALSE;
					else if(t==T_WHILE_NOT) op.opcode=OP_GOTO_IF_TRUE;
					call_stack.push(stack_info(TOP_LOOP,pos));
					op.opcode=var(s);
					WRITE_OP();
					break;
				case T_REPEAT:
					call_stack.push(stack_info(TOP_REPEAT,pos));
					break;
				case T_UNTIL:
				case T_UNTIL_N:
					if(call_stack.empty() || call_stack.top().operation!=TOP_LOOP)
						throw string("UNTIL without REPEAT");
					if(t==T_UNTIL) op.opcode=OP_GOTO_IF_FALSE;
					else if(t==T_UNTIL_N) op.opcode=OP_GOTO_IF_TRUE;
					op.jump=call_stack.top().position;
					call_stack.pop();
					op.parameter=var(s);
					WRITE_OP();
					break;
				case T_END:
					if(call_stack.empty())
						throw string("Unexpected END");
					if(call_stack.top().operation!=TOP_LOOP
					   && call_stack.top().operation!=TOP_ELSE
					   && call_stack.top().operation!=TOP_IF)
						throw string("Unexpected END");
					SET_JUMP(call_stack.top().position);
					call_stack.pop();
					break;
				case T_EOF:
					op.opcode=OP_STOP;
					WRITE_OP();
					if(!call_stack.empty())
						throw string("Unexpected End Of File");
					finished=true;
					break;
				default:
					throw string("Internal Error");
			}
		}
		
	}
	catch(string &s) {
		exit_status=1;
		cerr<<s<<endl;
	}
	fclose(fin);
	fclose(fop);
}



int main(int argc,char *argv[])
{
	if(argc<3) {
		cout<<"Usage: compile decl_file_name template1 template2...\n";
	}
	
	if(!load_vars(argv[1],"resvars.h")) {
		return 1;
	}
	int i;
	for(i=2;i<argc;i++) {
		string fopcode;
		if(check_names(argv[i],fopcode)) {
			compile(argv[i],fopcode);	
		}
	}
	return exit_status;
}
