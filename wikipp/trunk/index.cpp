#include "index.h"
#include "wiki.h"
#include "utf8/utf8.h"

namespace apps {

using namespace dbixx;

index::index(wiki &w):
	master(w)
{
	wi.url_next.add("^/index",
		boost::bind(&index::display_index,this));
}

string index::index_url()
{
	return wi.root()+"/index/";
}

void index::display_index()
{
	data::toc c;
	ini(c);
	result res;
	wi.sql<<"SELECT slug,title FROM pages "
		"WHERE lang=? "
		"ORDER BY title ASC",locale,res;
	unsigned items=res.rows();
	unsigned items_left=items/3;
	unsigned items_mid=items*2/3;
	string letter="";
	row r;
	for(unsigned i=0;res.next(r);i++) {
		vector<data::toc::element> *v;
		if(i<items_left)
			v=&c.left_col;
		else if(i<items_mid)
			v=&c.middle_col;
		else 
			v=&c.right_col;

		string t,slug;
		r>>slug>>t;
		if(!t.empty() && utf8::is_valid(t.begin(),t.end()))
		{
			std::string::iterator p=t.begin();
			utf8::next(p,t.end());
			string l(t.begin(),p);
			if(letter!=l) {
				data::toc::element e;
				e.letter=l;
				v->push_back(e);
				letter=l;
			}
			data::toc::element e;
			e.title=t;
			e.url=wi.page.page_url(locale,slug);
			v->push_back(e);
		}
	}
	wi.render("toc",c);
}
}
