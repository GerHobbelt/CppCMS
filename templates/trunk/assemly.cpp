#include "renderer.h"

#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace tmpl;
using namespace tmpl::details;
using namespace std;

vector<string> texts;

map<string,uint32_t> variables;

uint32_t max_local;
uint32_t max_seq;

map<string,uint32_t> functions;
vector<uint32_t> entries;


vector<instruction> ops;
map<string,uint32_t> labels;
map<uint32_t,string> references;

	static boost::regex r_label("^\\s*(\\w+)\\s*:\\s*$");
	static boost::regex r_extern("^\\s*extern\\s+(\\w+)\\s*$");
	static boost::regex r_show("^\\s*show\\s+([a-zA-Z]\\w*|\\d+|[a-zA-Z]\\w*\\(\\d+\\))\\s*$");
	static boost::regex r_start_seq("^\\s*seqf\\s+([a-zA-Z]\\w*|\\d+|[a-zA-Z]\\w*\\(\\d+\\)),(\\d+),(\\w+)\\s*$");
	static boost::regex r_store("^\\sto\\s+([a-zA-Z]\\w*|\\d+|[a-zA-Z]\\w*\\(\\d+\\)),(\\d+)\\s*$");
	static boost::regex r_next_seq("^\\s*seqn\\s+(\\d+),(\\w+)\\s*$");
	static boost::regex r_istest("^\\s*(def|true)\\s+([a-zA-Z]\\w*|\\d+|[a-zA-Z]\\w*\\(\\d+\\))\\s*$");
	static boost::regex r_jmp("^\\s*jmp\\s+(t|f|u),(\\w+)\\s*$");
	static boost::regex r_call("^\\s*call\\s+(\\w+)\\s*$");
	static boost::regex r_call_ref("^\\s*callr\\s+([a-zA-Z]\\w*|\\d+|[a-zA-Z]\\w*\\(\\d+\\))\\s*$");
	static boost::regex r_ret("^\\s*ret\\s*$");
	static boost::regex r_inline("^\\s*inline\\s+'([^']*)'\\s*$");

	static boost::regex r_var_glob("^([a-zA-Z]\\w*)$");
	static boost::regex r_var_ref("^([a-zA-Z]\\w*)\\((\\d+)\\)$");
	static boost::regex r_var_loc("^(\\d+)$");

	static boost::regex r_comment("^\\s*(#.*)?$");


#ifndef max
#define max(a,b) ((a)>(b) ? (a) : (b) )
#endif
int cur_line=0;
int error=0;


string make_string(string inp)
{
	string tmp;
	char const *s=inp.c_str();
	tmp.reserve(strlen(s));
	while(*s) {
		if(*s=='\\') {
			if(s[1]=='x' && isxdigit(s[2])  && isxdigit(s[3])){
				char buf[3];
				buf[0]=s[2];
				buf[1]=s[3];
				buf[2]=0;
				int n;
				sscanf(buf,"%x",&n);
				tmp+=(char)(unsigned char)(n);
				s+=3;
			}
			else {
				cerr<<boost::format("Illegal escape at line %d\n") % cur_line;
				return tmp;
			}
		}
		else if((unsigned char)*s < ' ') {
			cerr<<boost::format("Illegal charrecter at line %d\n") % cur_line;
			return tmp;
		}
		else {
			tmp+=*s;
		}
		s++;
	}
	return tmp;
}

void update_ref()
{
	map<uint32_t,string>::iterator p;
	for(p=references.begin();p!=references.end();p++) {
		map<string,uint32_t>::iterator l;
		l=labels.find(p->second);
		if(l==labels.end()) {
			cerr<<boost::format("Unreferenced label `%s'\n") % p->second;
			error=1;
		}
		else {
			ops[p->first].jump=l->second;
		}
	}
}

long tolong(string s)
{
	return atol(s.c_str());
}


void setup_var_op(instruction &op,string t)
{
	boost::cmatch m;
	map<string,uint32_t>::iterator p;
	if(boost::regex_match(t.c_str(),m,r_var_glob)) {
		op.flag=0;
		uint32_t id;
		if((p=variables.find(m[1]))==variables.end()) {
			id=variables.size();
			variables[m[1]]=id;
		}
		else {
			id=p->second;
		}
		op.r0=id;
	}
	else if(boost::regex_match(t.c_str(),m,r_var_ref)) {
		op.flag=1;
		uint32_t id;
		if((p=variables.find(m[1]))==variables.end()) {
			id=variables.size();
			variables[m[1]]=id;
		}
		else {
			id=p->second;
		}
		op.r0=id;
		op.r1=tolong(m[2]);
		max_seq=max(max_seq,op.r1);
	}
	else if(boost::regex_match(t.c_str(),m,r_var_loc)) {
		op.flag=2;
		op.r0=tolong(m[1]);
	}
	else {
		throw std::runtime_error("Internal error - expression should match");
	}
}


void process_command(char const *line)
{
	instruction op={0};
	boost::cmatch m;
	if(boost::regex_match(line,m,r_label)) {
		if(labels.find(m[1])!=labels.end()) {
			error=true;
			cerr<<boost::format("Duplicate label `%s' at line %d\n")%m[1]%cur_line;
			return;
		}
		else {
			labels[m[1]]=ops.size();
			return;
		}
	}
	else if(boost::regex_match(line,m,r_extern)) {
		if(labels.find(m[1])!=labels.end()) {
			error=true;
			cerr<<boost::format("Duplicate label `%s' at line %d\n")%m[1]%cur_line;
			return;
		}
		else {
			labels[m[1]]=ops.size();
			functions[m[1]]=ops.size();
			return;
		}
	}
	else if(boost::regex_match(line,m,r_show)) {
		op.opcode=OP_DISPLAY;
		setup_var_op(op,m[1]);
	}
	else if(boost::regex_match(line,m,r_start_seq)) {
		op.opcode=OP_START_SEQ;
		setup_var_op(op,m[1]);
		op.r2=tolong(m[2]);
		max_seq=max(max_seq,op.r2);
		references[ops.size()]=m[3];
	}
	else if(boost::regex_match(line,m,r_store)) {
		op.opcode=OP_STORE;
		setup_var_op(op,m[1]);
		op.r2=tolong(m[2]);
		max_local=max(max_local,op.r2);
	}
	else if(boost::regex_match(line,m,r_next_seq)) {
		op.opcode=OP_NEXT_SEQ;
		op.r0=tolong(m[1]);
		max_seq=max(max_seq,op.r0);
		references[ops.size()]=m[2];
	}
	else if(boost::regex_match(line,m,r_istest)) {
		if(string("def")==m[1])
			op.opcode=OP_CHECK_DEF;
		else
			op.opcode=OP_CHECK_TRUE;
		setup_var_op(op,m[2]);
	}
	else if(boost::regex_match(line,m,r_jmp)) {
		op.opcode=OP_JMP;
		switch(string(m[1])[0]) {
			case 'u':	op.flag=0; break;
			case 't':	op.flag=1; break;
			case 'f':	op.flag=2; break;
		}
		references[ops.size()]=m[2];
	}
	else if(boost::regex_match(line,m,r_call)) {
		op.opcode=OP_CALL;
		references[ops.size()]=m[1];
	}
	else if(boost::regex_match(line,m,r_call_ref)) {
		op.opcode=OP_CALL_REF;
		setup_var_op(op,m[1]);
	}
	else if(boost::regex_match(line,m,r_ret)){
		op.opcode=OP_RET;
	}
	else if(boost::regex_match(line,m,r_inline)){
		op.opcode=OP_INLINE;
		string tmp=make_string(m[1]);
		op.r0=texts.size();
		op.r1=tmp.size();
		texts.push_back(tmp);
	}
	else if(boost::regex_match(line,m,r_comment)){
		return;
	}
	else {
		cerr<<boost::format("Syntax error at line %d\n") % cur_line;
		return;
	}
	ops.push_back(op);
}

char const *names[] = { "inline", "display", "seq_start", "sto", "next" , "ch_def", "ch_true" ,
			"jmp", "call", "call_ref", "ret" };

bool read_line(string &s)
{
	int c;
	s="";
	while((c=getchar())!=EOF) {
		if(c=='\n')
			return true;
		s+=(char)c;
	}
	if(s!="")
		return true;
	return false;
}

FILE *fout;

void write_inverse(map<string,uint32_t> const &var)
{
	vector<string> tmp(var.size());
	map<string,uint32_t>::const_iterator p;
	for(p=var.begin();p!=var.end();p++) {
		tmp[p->second]=p->first;
	}
	unsigned i;
	for(i=0;i<tmp.size();i++) {
		fwrite(tmp[i].c_str(),tmp[i].size()+1,1,fout);
	}
}


void write_file(void)
{
	header h={0};
	memcpy(&h.magic,"TMPL",4);
	h.version=TMPL_VERSION;
	
	h.opcode_start=sizeof(h);
	h.opcode_size=ops.size();

	fwrite(&h,sizeof(h),1,fout);
	

	unsigned i;

	for(i=0;i<ops.size();i++){
		instruction op=ops[i];
		fwrite(&op,sizeof(op),1,fout);
	}
	
	h.func_entries_tbl_start=ftell(fout);;
	
	map<string,uint32_t>::const_iterator p;

	for(p=functions.begin();p!=functions.end();p++) {
		fwrite(&p->second,sizeof(uint32_t),1,fout);
	}

	h.func_tbl_start=ftell(fout);
	h.func_tbl_size=functions.size();

	for(p=functions.begin();p!=functions.end();p++) {
		fwrite(p->first.c_str(),p->first.size()+1,1,fout);
	}

	h.names_tbl_start=ftell(fout);
	h.names_tbl_size=variables.size();
	write_inverse(variables);

	h.texts_tbl_start=ftell(fout);
	h.texts_tbl_size=texts.size();
	for(i=0;i<texts.size();i++)
		fwrite(texts[i].c_str(),texts[i].size()+1,1,fout);
	h.local_variables=max_local;
	h.local_sequences=max_seq;

	fseek(fout,0,SEEK_SET);
	fwrite(&h,sizeof(h),1,fout);
}


int main(int argc,char **argv)
{
	if(argc!=2) {
		cerr<<"Usage: assembly file.template\n";
		return 1;
	}

	fout=fopen(argv[1],"wb");
	if(!fout) {
		cerr<<"Failed to open file"<<argv[1]<<endl;
		return 1;
	}

	string line;
	while(read_line(line)) {
		cur_line++;
		process_command(line.c_str());
	}
	update_ref();
	if(error)
		return error;
	
	write_file();
#if 1
	unsigned i;
	for(i=0;i<ops.size();i++) {
		printf("%04x\t%s\t(%d),%d,%d,%d:%04x\n", i, names[ops[i].opcode], ops[i].flag,
			ops[i].r0 ,ops[i].r1,ops[i].r2, ops[i].jump);
	}
#endif
	fclose(fout);
	return error;
}
