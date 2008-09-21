#ifndef CPPCMS_BASEVIEW_H
#define CPPCMS_BASEVIEW_H

#include "worker_thread.h"
#include <boost/format.hpp>

namespace cppcms {

class base_view_impl {
protected:
	ostream *cout_ptr;
	worker_thread *base_worker;

	void set_worker(worker_thread *w)
	{
		base_worker=worker;
		cout_ptr=&worker->cout;
	};

	inline ostream &cout() { return *cout_ptr; };

	string escape(string const &s);
	template<typename T>
	string escape(T const &v)
	{
		ostringstream s;
		s<<v;
		return s.str();
	};
	string urlencode(string const &s);
	inline char const *gettext(char const *s)
	{
		return base_worker->gettext(s);
	};
	inline char const *ngettext(char const *s,char const *p,int n)
	{
		return base_worker->ngettext(s,p,n);
	};
	inline boost::format format(string const &f){
		boost::format frm(f);
		frm.exceptions(0);
		return frm;
	};
public:

	virtual ~base_view_impl();
};

template<typename W>
class base_view : public base_view_impl {
protected:
	W *worker;
	inline stream &get_cout() { return *cout_ptr; };
public:
	void set_worker(W *w) {
		worker=w;
		base_view_impl::set_worker(w);
	};


};

};


#endif
