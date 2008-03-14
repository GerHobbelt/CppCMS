#include "renderer.h"
#include <boost/format.hpp>

#define DEBUG

#ifdef DEBUG
#include <iostream>
static char const *names[] = { "inline", "display", "seq_start", "sto", "next" , "ch_def", "ch_true" ,
	"jmp", "call", "call_ref", "ret" };
#endif

using namespace std;

namespace tmpl {

boost::any const &renderer::any_value(details::instruction const &op,content const &c)
{
	static boost::any empty;

	content::const_iterator p;
	content const *tmp;

	switch(op.flag){
	case 0:
		tmp=&c;
		break;
	case 1:
		tmp=seq[op.r1].get();
		break;
	case 2:
		{
			boost::any const *tmp=local[op.r0];
			if(tmp==NULL)
				return empty;
			return *tmp;
		}
		break;
	default:
		throw tmpl_error((boost::format("Incorrect flag value=%d at PC=%d") % op.flag % pc).str());
	}
	
	cout<<"Variables of content\n";
	for(p=c.begin();p!=c.end();p++) {
		cout<<p->first<<endl;
	}
	cout<<"Looking for "<<id_to_name[op.r0]<<endl;
	
	p=c.find(id_to_name[op.r0]);
	if(p==c.end()) {
		cout<<"Not found\n";
		return empty;
	}
	return p->second;
}

string const &renderer::string_value(details::instruction const &op,content const &c)
{
	boost::any const &val=any_value(op,c);
	static string empty;
	if(val.type()==typeid(string)) {
		return boost::any_cast<string const &>(val);
	}
	return empty;
}



bool renderer::bool_value(details::instruction const &op,content const &c)
{
	boost::any const &val=any_value(op,c);
	if(val.type()==typeid(bool)) {
		return boost::any_cast<bool>(val);
	}
	return false;
}


void renderer::setup()
{
	char const * base = (char const *)(view->image);
	details::header const *h=(details::header const *)view->image;
	if(memcmp(&(h->magic),"TMPL",4)!=0 || h->version!=TMPL_VERSION) {
		throw tmpl_error("Failed to load image");
	}
	opcodes=(details::instruction const *)(base+h->opcode_start);


	code_len=h->opcode_size;
	unsigned i;

#ifdef DEBUG
	cout<<"Inst "<<sizeof(details::instruction)<<endl;
	cout<<"Code "<<code_len<<' '<<h->opcode_start<<endl;
	cout<<"Func "<<h->func_tbl_size<<' '<<h->func_tbl_start<<endl;
	cout<<"Name "<<h->names_tbl_size<<' '<<h->names_tbl_start<<endl;
	cout<<"Text "<<h->texts_tbl_size<<' '<<h->texts_tbl_start<<endl;
	for(i=0;i<code_len;i++) {
		details::instruction const *ops=opcodes;
		printf("%04x\t%s\t(%d),%d,%d,%d:%04x\n", i, names[ops[i].opcode], ops[i].flag,
			ops[i].r0 ,ops[i].r1,ops[i].r2, ops[i].jump);
	}
#endif // DEBUG

	char const *first=base+h->func_tbl_start;
	uint32_t const *entries=(uint32_t const *)base+h->func_entries_tbl_start;
	for(i=0;i<h->func_tbl_size;i++) {
#ifdef DEBUG
			cout<<"Functions: "<<first<<' '<<entries[i]<<endl;
#endif
		functions[first]=entries[i];
		first+=strlen(first)+1;
	}
	first=base+h->names_tbl_start;
	for(i=0;i<h->names_tbl_size;i++) {
#ifdef DEBUG
		cout<<"Name "<<i<<" "<<first<<endl;
#endif
		id_to_name.push_back(first);
		first+=strlen(first)+1;
	}
	first=base+h->texts_tbl_start;
	for(i=0;i<h->texts_tbl_size;i++) {
#ifdef DEBUG
		cout<<"Text "<<i<<" "<<first<<endl;
#endif
		texts.push_back(first);
		first+=strlen(first)+1;
	}
	
	local_variables=h->local_variables;
	local_sequences=h->local_sequences;
#ifdef DEBUG
	cout<<"Vars: "<<local_variables<<" Seq:"<<local_sequences<<endl;
#endif


	local.assign(local_variables,NULL);
	seq = boost::shared_array<details::sequence>(new details::sequence[local_sequences] );
}

void renderer::render(content const &c,std::string const &func,string &out)
{
	/* Reset state machine*/
	flag=false;
	pc=0;
	local.assign(local_variables,NULL);
	unsigned i;
	for(i=0;i<local_sequences;i++)
		seq[i].reset();

	while(!call_stack.empty()) call_stack.pop();

	map<string,uint32_t>::const_iterator p;
	if((p=functions.find(func))==functions.end()) {
		// Nothing to render
		return ;
	}
	pc=p->second;
	bool finish=false;
	while(!finish) {
		using namespace details;
		#ifdef DEBUG
		printf("%04x %s\n",pc,names[opcodes[pc].opcode]);
		#endif //
		if(pc>=code_len) {
			throw tmpl_error((boost::format("Call overflow: PC=%1%")%pc).str());
		}
		instruction op=opcodes[pc];
		pc++;
		switch(op.opcode) {
		case	OP_JMP:
			if(op.flag==0 || (op.flag==1 && flag) || (op.flag==2 && !flag)){
				pc=op.jump;
			}
			break;
		case	OP_CALL:
			call_stack.push(pc);
			pc=op.jump;
			break;
		case	OP_RET:
			if(call_stack.empty()){
				finish=true;
			}
			else {
				pc=call_stack.top();
				call_stack.pop();
			}
			break;
		case	OP_INLINE:
			out.append(texts[op.r0],op.r1);
			break;
		case	OP_DISPLAY:
			{
				string const &tmp=string_value(op,c);
				out.append(tmp);
			}
			break;
		case	OP_CHECK_DEF:
			flag=!(any_value(op,c).empty());
			break;
		case	OP_CHECK_TRUE:
			flag=bool_value(op,c);
			break;
		case	OP_STORE:
			local[op.r2]=&any_value(op,c);
			break;
		case	OP_CALL_REF:
			{
				if((p=functions.find(string_value(op,c)))==functions.end()) {
					call_stack.push(pc);
					pc=p->second;
				}
			}
			break;
		case	OP_START_SEQ:
			{
				seq[op.r2].set(any_value(op,c));
				if(!seq[op.r2].first()) {
					pc=op.jump;
				}
			};
			break;
		case	OP_NEXT_SEQ:
			{
				if(!seq[op.r0].next()) {
					pc=op.jump;
				}
			}
			break;
		default:
			throw tmpl_error((boost::format("Incorrect opcode %d at PC=%d")%op.opcode%pc).str());
		};
	}
}


namespace details {
void sequence::set(boost::any const &a)
{
	if(a.type()==typeid(content::list_t)) {
		lst=&boost::any_cast<content::list_t const&>(a);
		type=s_list;
	}
	else if(a.type()==typeid(content::vector_t)) {
		vec=&boost::any_cast<content::vector_t const&>(a);
		type=s_vector;
	}
	else if(a.type()==typeid(content::callback_ptr)) {
		callback=&boost::any_cast<content::callback_ptr const &>(a);
		type=s_callback;
		tmp_content.erase(tmp_content.begin(),tmp_content.end());
	}
	else {
		type=s_none;
	}
};

bool sequence::next()
{
	switch(type) {
	case s_list:
		lst_iterator++;
		if(lst_iterator!=lst->end()) {
			return true;
		}
		break;
	case s_vector:
		vec_iterator++;
		if(vec_iterator!=vec->end()) {
			return true;
		}
		break;
	case s_callback:
		{
			content::callback_ptr const &cb=*callback;
			tmp_content.erase(tmp_content.begin(),tmp_content.end());
			return (*cb)(tmp_content);
		}
		break;
	default:
		return false;
	}
	return false;
}

bool sequence::first()
{
	switch(type) {
	case s_list:
		lst_iterator=lst->begin();
		if(lst_iterator!=lst->end()) {
			return true;
		}
		break;
	case s_vector:
		vec_iterator=vec->begin();
		if(vec_iterator!=vec->end()) {
			return true;
		}
		break;
	case s_callback:
		{
			content::callback_ptr const &cb=*callback;
			tmp_content.erase(tmp_content.begin(),tmp_content.end());
			return (*cb)(tmp_content);
		}
		break;
	default:
		return false;
	}
	return false;
}

content const * sequence::get()
{
	switch(type) {
	case s_list:
		return &*lst_iterator;
		break;
	case s_vector:
		return &*vec_iterator;
	case s_callback:
		return &tmp_content;
	default:
		tmp_content.erase(tmp_content.begin(),tmp_content.end());
		return &tmp_content;
	}
}

void sequence::reset()
{
	type=s_none;
}

};

template_data::template_data(std::string const &fname)
{
	FILE *f=fopen(fname.c_str(),"rb");
	if(!f){
		throw tmpl_error("Failed to open file:"+fname);
	}
	fseek(f,0,SEEK_END);
	uint32_t len=ftell(f);
	fseek(f,0,SEEK_SET);
	image=malloc(len);
	if(!image) {
		fclose(f);
		throw tmpl_error("Out of memory");
	}
	if(fread(image,1,len,f)!=len) {
		fclose(f);
		free(image);
		image=NULL;
		throw tmpl_error("Failed to read file:"+fname);
	}
	fclose(f);
}

template_data::~template_data()
{
	free(image);
}

};
