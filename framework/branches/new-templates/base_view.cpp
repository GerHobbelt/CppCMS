#include "base_view.h"
#include "worker_thread.h"

namespace cppcms {
void base_view_impl::set_worker(worker_thread *w)
{
	base_worker=w;
	cout_ptr=&w->cout;
};
char const *base_view_impl::gettext(char const *s)
{
	return base_worker->gettext(s);
};

char const *base_view_impl::ngettext(char const *s,char const *p,int n)
{
	return base_worker->ngettext(s,p,n);
};

namespace details {

views_storage &views_storage::instance() {
	static views_storage this_instance;
	return this_instance;
};

void views_storage::add_view(string t,string v,view_factory_t f)
{
	storage[t][v]=f;
}

void views_storage::remove_views(string t)
{
	storage.erase(t);
}

base_view_impl *views_storage::fetch_view(string t,string v)
{
	templates_t::iterator p=storage.find(t);
	if(p==storage.end()) return NULL;
	template_views_t::iterator p2=p->second.find(v);
	if(p2==p->second.end()) return NULL;
	view_factory_t &f=p2->second;
	return f();
}

};
}// CPPCMS
