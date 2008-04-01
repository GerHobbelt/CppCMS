#ifndef RENDERER_H
#define RENDERER_H
#include <stack>
#include <stdexcept>
#include <boost/shared_array.hpp>
#include <vector>
#include "content.h"

namespace tmpl {

#define TMPL_VERSION 0

class tmpl_error : public std::runtime_error
{
public:
        tmpl_error(std::string const &error) : std::runtime_error(error) {};
};

namespace details {

typedef enum {
	FLT_DEFAULT = 0, // string  - text2html
			 // std::tm - ISO time 2007-12-31 23:34:43
			 // int	    - as is
			 // bool    - none
	FLT_CHAIN,	 // use chain

	FLT_INTERNAL,

	FLT_STR_FILTER_FIRST,
	FLT_TO_HTML,	 // string to html
	FLT_RAW,
	FLT_STR_FILTER_LAST,
	FLT_DATE,	 // std::tm - 2007-12-31
	FLT_TIME,	 // std::tm   23:32
	FLT_TIME_SEC,	 // std::tm   23:32:43
	FLT_TIMEF,	 // std::tm   use format
			 // std::tm   local time presentation with seconds
	FLT_PRINTF,

	FLT_EXTERNAL = 32768,
			 // boost::any - external filter (by name)
			 // 		id-FLT_EXTERNAL=name
};

typedef enum {
	OP_INLINE,	// 		r0 text_id
			//		r1 text_len

	OP_DISPLAY,	// flag = 0,	r0 global variable 	print global_var[name(r0)]
			// flag = 1,	r0 -  sequence 		print local_seq[r1][name(r0)]
			//		r1 - sequence id 	
			// flag = 2,    r0 - input parameter	print local_var[r0]
			// 		r2 - filter ID 
			// 		jmp - filter parameter

	OP_START_SEQ,	// flag/r0/r1 - see display
			//		r2 - new seq id	local_seq[r2]=RES
			// 		jump - goto on empty

	OP_STORE,	// flag/r0/r1 - see display
			//		r2 local ref local_var[r2]=RES

	OP_NEXT_SEQ,	//		r0 sequence id		local_seq[r0]++;
			//		jump - goto NOT empty
	OP_CHECK_DEF,	// (flag/r0/r1)
	OP_CHECK_TRUE,	// (flag/r0/r1)
	OP_JMP,		// flag = 0	goto jump
			// flag = 1	jump on true
			// flag = 2	jump on false
	OP_CALL,	// 		call jump
	OP_CALL_REF,	// (flag/r0/r1)
	OP_RET,		//		return
	OP_PUSH_CHAIN,	// r0 - filter_id
			// jmp	filter_param
			//
};

struct instruction {
	uint8_t opcode;
	uint8_t flag;
	uint16_t r0;
	uint16_t r1;
	uint16_t r2;
	uint32_t jump;
};

struct header {
	uint32_t magic;
	uint32_t version;
	uint32_t opcode_start;
	uint32_t opcode_size;
	uint32_t func_tbl_start;
	uint32_t func_tbl_size;
	uint32_t func_entries_tbl_start;
	uint32_t names_tbl_start;
	uint32_t names_tbl_size;
	uint32_t texts_tbl_start;
	uint32_t texts_tbl_size;
	uint32_t local_variables;
	uint32_t local_sequences;
};



class sequence {
	content::list_t const *lst;
	content::list_t::const_iterator lst_iterator;
	content::vector_t const *vec;
	content::vector_t::const_iterator vec_iterator;
	content::callback_ptr const *callback;
	enum { s_none, s_list, s_vector, s_callback } type;
	content tmp_content;
public:
	sequence() : type(s_none) {};
	content const *get();
	bool next();
	bool first();
	void set(boost::any const &a);
	void reset();
};

}

class template_data {
public:
	void load(std::string const &fname);
	template_data(std::string const &fname) :image(NULL) { load(fname); };
	template_data() : image(NULL) {} ;
	~template_data();
	void *image;
};

class renderer 
{
	template_data const *view;

	std::vector<std::string> id_to_name;
	std::map<std::string,uint32_t> functions;
	uint32_t local_variables;
	uint32_t local_sequences;

	std::vector<char const *> texts;
	// Registers:
	uint32_t	pc;
	bool		flag;
	// Memory
	uint32_t code_len;
	details::instruction const *opcodes;
	// Sequence readers
	boost::shared_array<details::sequence> seq;
	// Local variables
	std::vector<boost::any const *> local;
	// Call Stack
	std::stack<uint32_t> call_stack;

	boost::any const &any_value(details::instruction const &op,content const &c);
	std::string const &string_value(details::instruction const &op,content const &c);
	bool bool_value(details::instruction const &op,content const &c);
	void setup();

public:
	typedef boost::signal<void (boost::any const &val,std::string &out)> converter_t;
	typedef boost::shared_ptr<converter_t>	converter_ptr;
	typedef boost::signal<void (boost::any const &a,std::string &out,char const *)> any_filter_t;
	typedef boost::signal<void (std::string const &a,std::string &out,char const *)> str_filter_t;
private:
	
	struct filter_data {
		uint16_t filter;
		uint16_t parameter;
		filter_data(uint16_t f=0,uint16_t p=0) : filter(f),parameter(p) {};
	};
	class type_holder {
		std::type_info const *typeinfo;
		converter_ptr	converter;
	public:
		type_holder() : typeinfo(&typeid(void)) {};
		type_holder(std::type_info const &t,converter_t::slot_type slot)
			: typeinfo(&t), converter(new converter_t) { converter->connect(slot); };
		std::type_info const &type() const { return *typeinfo; };
		void exec(boost::any const &val,std::string &out) const { (*converter)(val,out); };
	};
	typedef	std::list<type_holder> converters_list_t;
	converters_list_t converters;
	void	display(boost::any const &param,std::string &out,uint16_t filter,uint16_t param);

	class filter {
	public:
		bool string_input;	
		any_filter_t any_filter;
		str_filter_t str_filter;
	};

	typedef boost::shared_ptr<filter> filter_ptr;
	std::map<std::string,filter_ptr> external_filters;
	std::vector<filter_data> chain;
	
	void external_str_filter(std::string const &s,std::string &out,uint16_t filter,uint16_t param);
	void external_any_filter(boost::any const &a,std::string &out,uint16_t filter,uint16_t param);
	char const *get_parameter(uint16_t param);
	bool get_external_filter(filter_ptr &p,uint16_t filter);
	void str_filter(std::string const &str,std::string &out,uint16_t filter,uint16_t param);
	void any_filter(boost::any const &a,std::string &out,uint16_t filter,uint16_t param);
	void internal_time_filter(std::tm const &t,std::string out,uint16_t filter,uint16_t param);
	void internal_string_filter(std::string const &s,std::string out,uint16_t filter,uint16_t param);
	void internal_int_filter(int val,std::string out,uint16_t filter,uint16_t param);
	void default_filter(boost::any const &a,std::string &out);
	void text2html(std::string const &str,std::string &content);


public:
	renderer(template_data const &tmpl) : view(&tmpl) { setup(); };
	void render(content const &c,std::string const &func,std::string &out);
	void add_converter(std::type_info const &type,converter_t::slot_type slot);
	void add_string_filter(std::string const &name,str_filter_t::slot_type slot);
	void add_any_filter(std::string const &name,any_filter_t::slot_type slot);
};

}


#endif
