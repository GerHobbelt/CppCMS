#ifndef TMPL_LAMBDA_H
#define TMPL_LAMBDA_H

#include <map>
#include <string>

namespace transtext {
using namespace std;

namespace lambda {
	struct plural { // INTERFACE 
		virtual int operator()(int n) const = 0;
		virtual ~plural(){};
	};
	plural *compile(char const *expr);

}; // END OF NAMESPACE LAMBDA

class trans {
public:
	trans() {};
	virtual ~trans() {};
	virtual void load(char const * locale,char const *domain_name, char const * dirname) {};
	virtual char const *gettext(char const *s) const { return s; };
	virtual char const *ngettext(char const *s,char const *p,int n) const { return n==1 ? s:p; };
	char const *operator()(char const *str) const { return gettext(str); };
	char const *operator()(char const *single,char const *plural,int n) const { return ngettext(single,plural,n); };
};

class trans_thread_safe : public trans {
	lambda::plural *converter;
	void *data;
public:
	trans_thread_safe();
	virtual ~trans_thread_safe();
	virtual void load(char const * locale,char const *domain_name, char const * dirname);
	virtual char const *gettext(char const *s) const;
	virtual char const *ngettext(char const *single,char const *plural,int n) const;
	lambda::plural const & num2idx_conv() const { return *converter; };
	int num2idx(int n) const { return (*converter)(n); };
};

class trans_gnu: public trans {
public:
	trans_gnu() {};
	virtual ~trans_gnu() {};
	virtual void load(char const * locale,char const *domain_name, char const * dirname);
	virtual char const *gettext(char const *s) const;
	virtual char const *ngettext(char const *s,char const *p,int n) const;
	
};

class trans_factory {
	map<string,trans *> langs;
	map<string,string>  names;
	string def;
public:
	trans const &get() const;
	trans const &get(string const &lang) const;
	trans const &operator[](string const &lang) const { return get(lang); };
	void load(string const &locale_list,string const &domain,string const &dir,string const &d="");
	map<string,string> const &get_names() const { return names; };
	~trans_factory();
};

}


#endif
