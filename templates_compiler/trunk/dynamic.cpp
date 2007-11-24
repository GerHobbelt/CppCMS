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

void compile(char *fin_name,char const  *fop_name)
{
	FILE *fin=NULL,*fop=NULL;
	try{
		if((fin=fopen(fin_name,"r"))==NULL)
			throw string("Failed to open ")+fin_name;
		if((fop=fopen(fop_name,"wb"))==NULL)
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
				case T_INCLUDE_REF:
					op.opcode=OP_INCLUDE_REF;
					op.parameter=var(s);
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
					op.parameter=var(s);
					WRITE_OP();
					break;
				case T_REPEAT:
					call_stack.push(stack_info(TOP_REPEAT,pos));
					break;
				case T_UNTIL:
				case T_UNTIL_N:
					if(call_stack.empty() || call_stack.top().operation!=TOP_REPEAT)
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
					if(call_stack.top().operation==TOP_LOOP){
						op.opcode=OP_GOTO;
						op.jump=call_stack.top().position;
						WRITE_OP();
					}
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

void help()
{
	cerr<<"usage: -d interface.def [-p PREFIX ] [-h header.h] "
			"[-o template.op ] template.tmpl\n";
}

int main(int argc,char *argv[])
{
	string prefix="TMPL_";
	char *interface=NULL;
	char *interface_h=NULL;
	string s_op;
	char const *tmpl_op=NULL;
	char *tmpl=NULL;
	int i;
	for(i=1;i<argc;i++){
		if(strcmp(argv[i],"-i")==0 && i+1<argc && !interface) {
			i++;
			interface=argv[i];
		}
		else if(strcmp(argv[i],"-p")==0 && i+1<argc && prefix=="TMPL_") {
			i++;
			prefix=argv[i];
		}
		else if(strcmp(argv[i],"-h")==0 && i+1<argc && !interface_h) {
			i++;
			interface_h=argv[i];
		}
		else if(strcmp(argv[i],"-o")==0 && i+1<argc && !tmpl_op) {
			i++;
			tmpl_op=argv[i];
		}
		else if(tmpl==NULL) {
			tmpl=argv[i];
		}
		else {
			help();
			return 1;
		}
		
	}
	if(interface==NULL || (tmpl==NULL && interface_h==NULL)) {
		help();		
	}

	if(!load_vars(interface,interface_h,prefix.c_str())) {
		return 1;
	}

	
	if(tmpl_op==NULL && tmpl!=NULL) {
		if(!check_names(tmpl,s_op)) {
			help();
		}
		tmpl_op=s_op.c_str();
	}

	if(tmpl){
		compile(tmpl,tmpl_op);	
	}
	
	return exit_status;
}
