#ifndef CPPCMS_BASEVIEW_H
#define CPPCMS_BASEVIEW_H

#include <boost/format.hpp>
#include <boost/function.hpp>
#include <ostream>
#include <sstream>
#include <string>
#include <map>
#include "cppcms_error.h"

namespace cppcms {
using namespace std;
class worker_thread;
class base_view_impl {
protected:
	ostream *cout_ptr;
	worker_thread *base_worker;

	void set_worker(worker_thread *w);
	inline ostream &cout() { return *cout_ptr; };

	template<typename T>
	string escape(T const &v)
	{
		ostringstream s;
		s<<v;
		return s.str();
	};

	string escape(string const &s);

	string urlencode(string const &s);

	char const *gettext(char const *s);
	char const *ngettext(char const *s,char const *p,int n);

	inline boost::format format(string const &f){
		boost::format frm(f);
		frm.exceptions(0);
		return frm;
	};
public:
	virtual void render() = 0;

	virtual ~base_view_impl() {};
};

template<typename W>
class base_view : public base_view_impl {
protected:
	W *worker;
	inline std::ostream &get_cout() { return *cout_ptr; };
public:
	void set_worker(worker_thread *w) {
		worker=dynamic_cast<W*>(w);
		if(!worker) throw cppcms_error("Can't set_worker on class not derived from cppcms::worker_thread");
		base_view_impl::set_worker(w);
	};

	base_view() {};
	base_view(W *p) {worker=p; base_view_impl::set_worker(p); };
};

namespace details {

template<typename T>
struct view_builder {
        base_view_impl *operator()() { return new T; };
};

class views_storage {
public:
	typedef boost::function<base_view_impl *()> view_factory_t;
private:
	typedef map<string,view_factory_t> template_views_t;
	typedef map<string,template_views_t> templates_t;

	templates_t storage;
public:

	void add_view(	string template_name,
			string view_name,
			view_factory_t);
	void remove_views(string template_name);
	base_view_impl *fetch_view(string template_name,string view_name);
	static views_storage &instance();
};

}; // DETAILS


}; // CPPCMS

#define cppcms_master_view(X,Y) X: virtual public cppcms::base_view< Y >
#define cppcms_extend_view(X,Y) X: virtual public Y

#endif
