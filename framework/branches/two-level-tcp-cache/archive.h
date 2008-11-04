#ifndef ARCHIVE_H
#define ARCHIVE_H
#include "cppcms_error.h"
#include <string>
#include <cstring>

namespace cppcms {

using namespace std;

class archive {
	string data;
	uint32_t ptr;
public:
	archive() { ptr=0; };
	archive(string const &s) : data(s) { ptr=0; };
	void set(string const &s) { data=s; ptr=0; };
	void set(char const *ptr,uint32_t len) { data.assign(ptr,len); };
	string const &get() const { return data; };
	template<typename T>
	archive &operator<<(T const &val) {
		uint32_t size=sizeof(T);
		data.append((char const *)&size,4);
		data.append((char const *)&val,size);
		return *this;
	}
	archive &operator<<(string const &val) {
		uint32_t size=val.size();
		data.append((char const *)&size,4);
		data.append(val.c_str(),size);
		return *this;
	}
	template<typename T>
	archive &operator>>(T &val)
	{
		if(ptr+4+sizeof(T)>data.size()) {
			throw cppcms_error("Format violation");
		}
		char const *start=data.c_str()+ptr;
		if(*(uint32_t const *)start!=sizeof(T)) {
			throw cppcms_error("Invalid size read");
		}
		start+=4;

		memcpy(&val,start,sizeof(T));

		ptr+=4+sizeof(T);
		return *this;
	}
	archive &operator>>(string &val)
	{
		if(ptr+4>data.size()) {
			throw cppcms_error("Format violation");
		}
		char const *start=data.c_str()+ptr;
		uint32_t s=*(uint32_t const *)start;
		if(ptr+4+s>data.size()) {
			throw cppcms_error("String too long");
		}
		start+=4;
		val.assign(start,s);
		ptr+=4+s;
		return *this;
	}
	void skip()
	{
		if(ptr+4>data.size()) {
			throw cppcms_error("Format violation");
		}
		char const *start=data.c_str()+ptr;
		uint32_t s=*(uint32_t const *)start;
		if(ptr+4+s>data.size()) {
			throw cppcms_error("String too long");
		}
		ptr+=4+s;
	}
};

class serializable {
public:
	virtual void load(archive &a) = 0;
	virtual void save(archive &a) const = 0;
	virtual ~serializable() {};
};
}

#endif
