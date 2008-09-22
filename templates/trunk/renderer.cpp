#include "renderer.h"
#include <boost/format.hpp>
#include <iostream>

//#define DEBUG

#ifdef DEBUG
#include <iostream>
static char const *names[] = { "inline", "display", "seq_start", "sto", "next" , "ch_def", "ch_true" ,
	"jmp", "call", "call_ref", "ret","showf","gettext","ngettext","ch_rtl" };
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
		if(op.r1>=local_sequences) 
			return empty;
		tmp=seq[op.r1].get();
		break;
	case 2:
		{
			if(op.r0>=local_variables){
				return empty;
			}
			boost::any const *tmp=local[op.r0];
			if(tmp==NULL)
				return empty;
			return *tmp;
		}
		break;
	default:
		throw tmpl_error((boost::format("Incorrect flag value=%d at PC=%d") % op.flag % pc).str());
	}
	
	p=tmp->find(id_to_name[op.r0]);
	if(p==tmp->end()) {
		return empty;
	}
	return p->second;
}

void renderer::add_converter(std::type_info const &type,converter_t slot)
{
	converters.push_back(type_holder(type,slot));
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
	cout<<"Entr "<<h->func_entries_tbl_start<<endl;
	cout<<"Name "<<h->names_tbl_size<<' '<<h->names_tbl_start<<endl;
	cout<<"Text "<<h->texts_tbl_size<<' '<<h->texts_tbl_start<<endl;
	for(i=0;i<code_len;i++) {
		details::instruction const *ops=opcodes;
		printf("%04x\t%s\t(%d),%d,%d,%d:%04x\n", i, names[ops[i].opcode], ops[i].flag,
			ops[i].r0 ,ops[i].r1,ops[i].r2, ops[i].jump);
	}
#endif // DEBUG

	char const *first=base+h->func_tbl_start;
	uint32_t const *entries=(uint32_t const *)(base+h->func_entries_tbl_start);
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
	chain.reserve(10);
}

void renderer::create_formated_string(string const &str,string &out,int const &n)
{
	unsigned i;
	for(i=0;i<str.size();i++) {
		if(str[i]=='%' && str[i+1]=='%') {
			out+='%';
			i++;
		}
		else if(str[i]=='%' && (str[i+1]=='d' || str[i+1]=='i')) {
			char buf[16];
			snprintf(buf,sizeof(buf),"%d",n);
			out.append(buf);
			i++;
		}
		else if(str[i]=='%' && isdigit(str[i+1])) {
			char const *ptr=str.c_str()+i+1;
			char *ptr2;
			int pos=strtol(ptr,&ptr2,10)-1; // Index starts from 1
			if(*ptr2!='%') continue;
			i+=ptr2-ptr+1;
			if(pos>=0 && pos<(int)format_strings.size()) {
				out.append(format_strings[pos]);
			}
		}
		else {
			out+=str[i];
		}
	}
	format_strings.clear();
	format_strings.reserve(10);
}



void renderer::render(content const &c,std::string const &func,string &out)
{
	/* Reset state machine*/
	static const transtext::trans default_tr;

	transtext::trans const &tr=current_tr ? *current_tr : default_tr;

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
			display(any_value(op,c),out,op.r2,op.jump);
			break;
		case	OP_CHECK_DEF:
			flag=!(any_value(op,c).empty());
			break;
		case	OP_CHECK_TRUE:
			flag=bool_value(op,c);
			break;
		case	OP_CHECK_RTL:
			flag=(strcmp(tr("LTR"),"RTL") == 0);
			break;
		case	OP_CHECK_EQ:
			flag=false;
			if(op.r0<=local_variables && op.r1<=local_variables) {
				boost::any const *a1,*a2;
				if((a1=local[op.r0])!=NULL && (a2=local[op.r1])!=NULL) {
					if(a1->type()==typeid(int) && a2->type()==typeid(int)) {
						int v1=boost::any_cast<int const &>(*a1);
						int v2=boost::any_cast<int const &>(*a2);
						flag=v1==v2;
					}
					else if(a1->type()==typeid(string) && a2->type()==typeid(string)) {
						flag=boost::any_cast<string const &>(*a1)==boost::any_cast<string const &>(*a2);
					}
					else if(a1->type()==typeid(bool) && a2->type()==typeid(bool)) {
						flag=boost::any_cast<bool const &>(*a1)==boost::any_cast<bool const &>(*a2);
					}
				}
			}
			break;
		case	OP_STORE:
			if(op.r2<=local_variables)
				local[op.r2]=&any_value(op,c);
			break;
		case	OP_CALL_REF:
			{
				if((p=functions.find(string_value(op,c)))!=functions.end()) {
#ifdef DEBUG				
					cout<<"Calling:"<<p->first<<endl;
#endif
					call_stack.push(pc);
					pc=p->second;
				}
			}
			break;
		case	OP_START_SEQ:
			{
				if(op.r2>=local_sequences){
					pc=op.jump;
					break;
				}
				seq[op.r2].set(any_value(op,c));
				if(!seq[op.r2].first()) {
					pc=op.jump;
				}
			};
			break;
		case	OP_NEXT_SEQ:
			{
				if(op.r0>=local_sequences || seq[op.r0].next()) {
					pc=op.jump;
				}
			}
			break;
		case 	OP_PUSH_CHAIN:
			chain.push_back(filter_data(op.r0,op.jump));
			break;
		case	OP_DISPLAYF:
			{
				string tmp;
				display(any_value(op,c),tmp,op.r2,op.jump);
				format_strings.push_back(tmp);
			}
			break;
		case	OP_GETTEXT:
			{
				create_formated_string(tr.gettext(texts[op.r0]),out);
			};
			break;
		case	OP_NGETTEXT:
			{
				if(any_value(op,c).type()!=typeid(int))
					break;
				int n=boost::any_cast<int>(any_value(op,c));
				create_formated_string(tr.ngettext(texts[op.r2],texts[op.jump],n),out,n);
			};
			break;
		default:
			throw tmpl_error((boost::format("Incorrect opcode %d at PC=%d")%op.opcode%pc).str());
		};
	}
}

void renderer::text2html(string const &str,string &content)
{
	int i;
	int len=str.size();
	for(i=0;i<len;i++) {
		char c=str[i];
		switch(c){
			case '<': content+="&lt;"; break;
			case '>': content+="&gt;"; break;
			case '&': content+="&amp;"; break;
			case '\"': content+="&quot;"; break;
			default: content+=c;
		}
	}
}

void renderer::usertype_to_string(boost::any const &a,string &out)
{
	converters_list_t::iterator p;
	for(p=converters.begin();p!=converters.end();p++){
		if(p->type()==a.type()) {
			p->exec(a,out);
			return;
		}
	}
}

void renderer::default_filter(boost::any const &a,string &out)
{
	if(a.type()==typeid(string)) {
		string const &tmp=boost::any_cast<string const &>(a);
		text2html(tmp,out);
	}
	else if(a.type()==typeid(int)) {
		out.append(str(boost::format("%d") % boost::any_cast<int>(a)));
	}
	else if(a.type()==typeid(std::tm)) {
		std::tm const &tmp=boost::any_cast<std::tm const &>(a);
		char buf[32];
		strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M",&tmp);
		out.append(buf);
	}
	else {
		usertype_to_string(a,out);
	}
	
}

void renderer::internal_string_filter(string const &s,string &out,uint16_t filter,uint16_t param)
{
	using namespace details;
	switch(filter) {
	case FLT_TO_HTML:
		text2html(s,out);
		break;
	case FLT_RAW:
		out.append(s);
		break;
	}
}

void renderer::add_string_filter(std::string const &name,str_filter_t slot)
{
	filter &f=external_filters[name];
	f.string_input=true;
	f.str_filter=slot;
}

void renderer::add_any_filter(std::string const &name,any_filter_t slot)
{
	filter &f=external_filters[name];
	f.string_input=false;
	f.any_filter=slot;
}

void renderer::internal_int_filter(int val,string &out,uint16_t filter,uint16_t param)
{
	char const *format;
	switch(filter){
	case details::FLT_PRINTF:
		format=get_parameter(param);
		break;
	default:
		format="%d";
	}
	if(format) {
		try{
			out.append(str(boost::format(format) % val));
		}
		catch(std::exception const &e)
		{ /* In case of bad format string */ }
	}
}

void renderer::internal_time_filter(std::tm const &t,string &out,uint16_t filter,uint16_t param)
{
	using namespace details;
	char const *format=NULL;
	switch(filter){
	case FLT_DATE:	format="%Y-%m-%d"; break;
	case FLT_TIME:	format="%H:%M"; break;
	case FLT_TIME_SEC: format="%T"; break;
	case FLT_TIMEF: format=get_parameter(param); break;
	}
	if(format) {
		char buf[128];
		strftime(buf,sizeof(buf),format,&t);
		out.append(buf);
	}
}

void renderer::any_filter(boost::any const &a,string &out,uint16_t filter,uint16_t param)
{
	using namespace details;

	if(filter>=FLT_INTERNAL && filter<FLT_EXTERNAL) {
		if(a.type()==typeid(string)) {
			internal_string_filter(boost::any_cast<string const &>(a),out,filter,param);
		}
		else if(a.type()==typeid(std::tm)){
			internal_time_filter(boost::any_cast<std::tm const &>(a),out,filter,param);
		}
		else if(a.type()==typeid(int)) {
			internal_int_filter(boost::any_cast<int>(a),out,filter,param);
		}
		else if(filter==FLT_RAW){
			usertype_to_string(a,out);
		}
	}
	else if(filter & FLT_EXTERNAL){
		external_any_filter(a,out,filter,param);
	}
}

void renderer::str_filter(string const &str,string &out,uint16_t filter,uint16_t param)
{
	using namespace details;
	if(filter>=FLT_STR_FILTER_FIRST && filter<=FLT_STR_FILTER_LAST) {
		internal_string_filter(str,out,filter,param);
	}
	else if(filter & FLT_EXTERNAL) {
		external_str_filter(str,out,filter,param);
	}
}

bool renderer::get_external_filter(renderer::filter *&p,uint16_t filter)
{
	using namespace details;
	if(filter < FLT_EXTERNAL || filter-FLT_EXTERNAL >= (int)id_to_name.size() )
		return false;
	string name=id_to_name[filter-FLT_EXTERNAL];
	map<string,renderer::filter>::iterator fp;
	if((fp=external_filters.find(name))!=external_filters.end()) {
		p=&fp->second;
		return true;
	}
	return false;
} 

char const *renderer::get_parameter(uint16_t param)
{
	if(param==0 || param>=texts.size()) {
		return NULL;
	}
	return texts[param];
}

void renderer::external_any_filter(boost::any const &a,string &out,uint16_t filter,uint16_t param)
{
	renderer::filter *fp;
	if(get_external_filter(fp,filter)) {
		if(fp->string_input) {
			if(a.type()==typeid(string)) {
				string const &tmp=boost::any_cast<string const &>(a);
				fp->str_filter(tmp,out,get_parameter(param));
			}
			else {
				string tmp;
				default_filter(a,tmp);
				fp->str_filter(tmp,out,get_parameter(param));
			}
		}
		else {
			fp->any_filter(a,out,get_parameter(param));
		}
	}
}

void renderer::external_str_filter(string const &s,string &out,uint16_t filter,uint16_t param)
{
	renderer::filter *fp;
	if(get_external_filter(fp,filter)) {
		if(fp->string_input) {
			fp->str_filter(s,out,get_parameter(param));
		}
		else{
			boost::any a;
			a=s;
			fp->any_filter(a,out,get_parameter(param));
		}
	}
}

void renderer::display(boost::any const &a,string &out,uint16_t filter,uint16_t parameter)
{
	using namespace details;
	int i;
	string tmp,tmp_out;
	
	switch(filter){
	case FLT_DEFAULT:
		default_filter(a,out);
		break;
	case FLT_CHAIN:
		if(chain.size()==0)
			return;
		if(chain.size()==1) {
			any_filter(a,out,chain[0].filter,chain[0].parameter);
		}
		else {
			any_filter(a,tmp,chain[0].filter,chain[0].parameter);
			for(i=1;i<(int)chain.size()-1;i++){
				str_filter(tmp,tmp_out,chain[i].filter,chain[i].parameter);
				tmp=tmp_out;
			}
			str_filter(tmp,out,chain[i].filter,chain[i].parameter);
		}
		chain.clear();
		chain.reserve(10);
		break;
	default:
		any_filter(a,out,filter,parameter);
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
	else if(a.type()==typeid(content::callback_t)) {
		callback=&boost::any_cast<content::callback_t const &>(a);
		type=s_callback;
		tmp_content.clear();
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
			content::callback_t const &cb=*callback;
			tmp_content.clear();
			if(cb(tmp_content)) {
				return true;
			}
		}
		break;
	default:
		return false;
	}
	type=s_none;
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
			content::callback_t const &cb=*callback;
			tmp_content.clear();
			if(cb(tmp_content)){
				return true;
			}
		}
		break;
	default:
		return false;
	}
	type=s_none;
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
		tmp_content.clear();
		return &tmp_content;
	}
}

void sequence::reset()
{
	type=s_none;
}

};

void template_data::load(std::string const &fname)
{
	if(image){
		throw tmpl_error("Can't load template twice");
	}
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
