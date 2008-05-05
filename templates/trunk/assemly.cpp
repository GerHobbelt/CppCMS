#include "renderer.h"

#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <iostream>
#include <set>
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
	static boost::regex r_show("^\\s*show\\s+([a-zA-Z]\\w*|\\d+|[a-zA-Z]\\w*\\(\\d+\\))(,((ext\\s)?\\w+(,.*)?))?\\s*$");
	static boost::regex r_showf("^\\s*showf\\s+([a-zA-Z]\\w*|\\d+|[a-zA-Z]\\w*\\(\\d+\\))(,((ext\\s)?\\w+(,.*)?))?\\s*$");
	static boost::regex r_start_seq("^\\s*seqf\\s+([a-zA-Z]\\w*|\\d+|[a-zA-Z]\\w*\\(\\d+\\)),(\\d+),(\\w+)\\s*$");
	static boost::regex r_store("^\\s*sto\\s+([a-zA-Z]\\w*|\\d+|[a-zA-Z]\\w*\\(\\d+\\)),(\\d+)\\s*$");
	static boost::regex r_next_seq("^\\s*seqn\\s+(\\d+),(\\w+)\\s*$");
	static boost::regex r_istest("^\\s*(def|true)\\s+([a-zA-Z]\\w*|\\d+|[a-zA-Z]\\w*\\(\\d+\\))\\s*$");
	static boost::regex r_jmp("^\\s*jmp\\s+(t|f|u),(\\w+)\\s*$");
	static boost::regex r_call("^\\s*call\\s+(\\w+)\\s*$");
	static boost::regex r_call_ref("^\\s*callr\\s+([a-zA-Z]\\w*|\\d+|[a-zA-Z]\\w*\\(\\d+\\))\\s*$");
	static boost::regex r_ret("^\\s*ret\\s*$");
	static boost::regex r_inline("^\\s*inline\\s+'([^']*)'\\s*$");
	static boost::regex r_gettext("^\\s*gt\\s+'([^']*)'\\s*$");
	static boost::regex r_ngettext("^\\s*ngt\\s+([a-zA-Z]\\w*|\\d+|[a-zA-Z]\\w*\\(\\d+\\)),'([^']*)','([^']*)'\\s*$");
	static boost::regex r_rtl("^\\s*rtl\\s*$");


	static boost::regex r_var_glob("^([a-zA-Z]\\w*)$");
	static boost::regex r_var_ref("^([a-zA-Z]\\w*)\\((\\d+)\\)$");
	static boost::regex r_var_loc("^(\\d+)$");

	static boost::regex r_comment("^\\s*(#.*)?$");
	static boost::regex r_filter("^\\s*filter\\s+((ext\\s)?\\w+(,.*)?)\\s*$");


#ifndef max
#define max(a,b) ((a)>(b) ? (a) : (b) )
#endif
int cur_line=0;
int error=0;

FILE *pot_file=NULL;
set<string> xgotten_strings;

string potstring(string in)
{
	unsigned i;
	char buf[8];
	string result;
	for(i=0;i<in.size();i++) {
		char c=in[i];
		char const *tmp=NULL;
		switch(c) {
		case '\n': 
			if(i==in.size()-1)
				tmp="\\n";
			else
				tmp="\\n\"\n\""; 
			break;
		case '\t': tmp="\\t"; break;
		case '\v': tmp="\\v"; break;
		case '\b': tmp="\\b"; break;
		case '\r': tmp="\\r"; break;
		case '\f': tmp="\\f"; break;
		case '\a': tmp="\\a"; break;
		case '\\': tmp="\\\\"; break;
		case '\"': tmp="\\\""; break;
		default:
			if(c>=0 && c<32) {
				snprintf(buf,sizeof(buf),"\\x%02x",c);
				tmp=buf;
			}
		}
		if(tmp)
			result+=tmp;
		else
			result+=c;
	}
	return result;
}

void xgettext(string s)
{
	if(!pot_file) return;
	if(xgotten_strings.find(s)!=xgotten_strings.end()) return;
	fprintf(pot_file,"msgid \"%s\"\nmsgstr \"\"\n\n\n",potstring(s).c_str());
	xgotten_strings.insert(s);
}

void xngettext(string s,string p)
{
	if(!pot_file) return;
	if(xgotten_strings.find(s)!=xgotten_strings.end()) return;
	fprintf(pot_file,"msgid \"%s\"\nmsgid_plural \"%s\"\nmsgstr[0] \"\"\nmsgstr[1] \"\"\n\n\n",
			potstring(s).c_str(),potstring(p).c_str());
	xgotten_strings.insert(s);
}

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
				if(n==0) {
					cerr<<"Zero charrecter is illegal in line "<<cur_line;
					return tmp;
				}
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

uint32_t get_variable(string const &name)
{
	uint32_t id;
	map<string,uint32_t>::iterator p;
	if((p=variables.find(name))==variables.end()) {
		id=variables.size();
		variables[name]=id;
	}
	else {
		id=p->second;
	}
	return id;
}


void setup_var_op(instruction &op,string t)
{
	boost::cmatch m;
	map<string,uint32_t>::iterator p;
	if(boost::regex_match(t.c_str(),m,r_var_glob)) {
		op.flag=0;
		uint32_t id=get_variable(m[1]);
		op.r0=id;
	}
	else if(boost::regex_match(t.c_str(),m,r_var_ref)) {
		op.flag=1;
		uint32_t id=get_variable(m[1]);
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

uint16_t get_param(string const &s)
{
	string tmp=make_string(s);
	if(texts.size()==0) {
		// param must not be 0
		texts.push_back(string(""));
	}
	uint16_t res=texts.size();
	texts.push_back(tmp);
	return res;
}

void setup_filter(string const &s,uint16_t &filter,uint32_t &param)
{
	static boost::regex external("^ext\\s+([a-zA-Z]\\w*)(,'([^']*)')?\\s*$");
	static boost::regex internal("^([a-zA-Z]\\w*)(,'([^']*)')?\\s*$");
	
	boost::cmatch m;
	filter=0;
	param=0;
	if(boost::regex_match(s.c_str(),m,external)) {
		uint16_t id=get_variable(m[1]);
		filter=FLT_EXTERNAL+id;
	}
	else if(boost::regex_match(s.c_str(),m,internal)){
		static struct { char const *name; uint16_t id; } 
			data[] = 
			{
				{ "chain" , FLT_CHAIN },
				{ "escape", FLT_TO_HTML},
				{ "raw", FLT_RAW },
				{ "date", FLT_DATE },
				{ "time", FLT_TIME },
				{ "timesec", FLT_TIME_SEC },
				{ "strftime", FLT_TIMEF },
				{ "intf", FLT_PRINTF },
			};
		int i,n=sizeof(data)/sizeof(data[0]);
		for(i=0;i<n;i++) {
			if(m[1]==data[i].name) {
				filter=data[i].id;
				break;
			}
		}
		if(i==n) {
			error=true;
			cerr<<boost::format("Unknown filter %s at line %d\n") % m[1] % cur_line;
		}
	}
	else {
		error=true;
		cerr<<"Syntax error in filter definiton `" << s <<"'at line "<<cur_line<<endl;
	}
	if(m[2]!="") {
		param=get_param(m[3]);
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
		if(m[2]!="") {
			setup_filter(m[3],op.r2,op.jump);
		}
	}
	else if(boost::regex_match(line,m,r_showf)) {
		op.opcode=OP_DISPLAYF;
		setup_var_op(op,m[1]);
		if(m[2]!="") {
			setup_filter(m[3],op.r2,op.jump);
		}
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
	else if(boost::regex_match(line,m,r_rtl)) {
		op.opcode=OP_CHECK_RTL;
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
	else if(boost::regex_match(line,m,r_filter)) {
		op.opcode=OP_PUSH_CHAIN;
		setup_filter(m[1],op.r0,op.jump);
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
	else if(boost::regex_match(line,m,r_gettext)){
		op.opcode=OP_GETTEXT;
		string tmp=make_string(m[1]);
		xgettext(tmp);
		op.r0=texts.size();
		texts.push_back(tmp);
	}
	else if(boost::regex_match(line,m,r_ngettext)){
		op.opcode=OP_NGETTEXT;
		
		setup_var_op(op,m[1]);

		string single=make_string(m[2]);
		op.r2=texts.size();
		texts.push_back(single);

		string plural=make_string(m[3]);
		op.jump=texts.size();
		texts.push_back(plural);

		xngettext(single,plural);
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
			"jmp", "call", "call_ref", "ret", "filter" };

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
	h.local_variables=max_local+1;
	h.local_sequences=max_seq+1;

	fseek(fout,0,SEEK_SET);
	fwrite(&h,sizeof(h),1,fout);
}


int main(int argc,char **argv)
{
	if(argc!=2 && argc!=3) {
		cerr<<"Usage: assembly file.template [file.pot]\n";
		return 1;
	}

	fout=fopen(argv[1],"wb");
	if(!fout) {
		cerr<<"Failed to open file"<<argv[1]<<endl;
		return 1;
	}

	if(argc==3) {
		pot_file=fopen(argv[2],"w");
		if(!pot_file) {
			cerr<<"Failed to open file"<<argv[2]<<endl;
			return 1;
		}

		fprintf(pot_file,"# Please translate as RTL for Right-to-Left languages like Hebrew or Arabic\n"
				"msgid \"LTR\"\nmsgstr \"\"\n\n");
		xgotten_strings.insert("LTR");
		
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
#ifdef DEBUG
	unsigned i;
	for(i=0;i<ops.size();i++) {
		printf("%04x\t%s\t(%d),%d,%d,%d:%04x\n", i, names[ops[i].opcode], ops[i].flag,
			ops[i].r0 ,ops[i].r1,ops[i].r2, ops[i].jump);
	}
#endif
	fclose(fout);
	if(pot_file) fclose(pot_file);
	return error;
}
