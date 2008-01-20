#ifndef REL_H
#define REL_H

#include <stdint.h>
#include <stdlib.h>

template<typename O,void **base_value_pointer>
class sh_pointer {
public:
	intptr_t ptr;
	char *base() { return (char*)*base_value_pointer; };
	typedef O *Op;
	typedef O &Or;
public:
	Or operator*() { return &(Op)(base()+ptr); };
	Op operator->() { return (Op)(base()+ptr); };
	Op get() { return (Op)(base()+ptr); };
	sh_pointer<O,base_value_pointer> &operator=(Op p) {
		if(p==NULL) ptr=0;
		else {
			ptr=(char*)p-base();
		}
		return *this;
	};
	sh_pointer<O,base_value_pointer> &operator=(sh_pointer<O,base_value_pointer> p) {
		ptr=p.ptr;
		return *this;
	};
	sh_pointer() {
		ptr=0;
	};
	bool operator==(void *v) {
		if(v==NULL) return ptr==0;
		return get()==v;
	};
	bool operator!=(void *v) {
		if(v==NULL) return ptr!=0;
		return get()!=v;
	};
	
	bool operator==(sh_pointer<O,base_value_pointer> p) {
		return ptr==p.ptr;
	};
	bool operator!=(sh_pointer<O,base_value_pointer> p) {
		return ptr!=p.ptr;
	};

};

#endif

