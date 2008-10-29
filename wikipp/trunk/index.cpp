#include "index.h"
#include "wiki.h"
#include "utf8/utf8.h"

namespace apps {

using namespace dbixx;

index::index(wiki &w):
	master(w)
{
	wi.url_next.add("^/index/?",
		boost::bind(&index::display_index,this));
	wi.url_next.add("^/changes(/?|/(\\d+))$",
		boost::bind(&index::changes,this,$2));
}

string index::index_url()
{
	return wi.root()+"/index/";
}

string index::changes_url(int p)
{
	if(p==0)
		return wi.root()+"/changes/";
	return wi.root()+(boost::format("/changes/%1%") % p).str();
}

void index::changes(string page_no)
{
	int p;
	const unsigned window=30;
	if(page_no.empty())
		p=0;
	else
		p=atoi(page_no.c_str());
	result rs;
	sql<<	"SELECT history.title,history.version,history.created,"
		"	history.author,pages.lang,pages.slug "
		"FROM history "
		"JOIN pages ON history.id=pages.id "
		"ORDER BY created DESC "
		"LIMIT ?,?",
		p*window,window,
		rs;
	data::recent_changes c;	
	c.data.resize(rs.rows());
	row r;
	int n;
	for(n=0;rs.next(r);n++) {
		data::recent_changes::element &d=c.data[n];
		string lang,slug;
		r>>d.title>>d.version>>d.created>>d.author>>lang>>slug;
		d.url=wi.page.page_version_url(d.version,lang,slug);
	}
	if(c.data.size()==window)
		c.next=changes_url(p+1);
	ini(c);
	render("recent_changes",c);
}

void index::display_index()
{
	data::toc c;
	ini(c);
	result res;
	sql<<	"SELECT slug,title FROM pages "
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
	render("toc",c);
}
}
