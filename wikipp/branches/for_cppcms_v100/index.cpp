#include "index.h"
#include "index_content.h"
#include "wiki.h"
#include "utf8/utf8.h"

#include <cppcms/cache_interface.h>
#include <cppcms/url_dispatcher.h>

namespace apps {

using namespace dbixx;

index::index(wiki &w):
	master(w)
{
	wi.dispatcher().assign("^/index/?",&index::display_index,this);
	wi.dispatcher().assign("^/changes(/?|/(\\d+))$",&index::changes,this,2);
}

std::string index::index_url()
{
	return wi.root()+"/index/";
}

std::string index::changes_url(int p)
{
	if(p==0)
		return wi.root()+"/changes/";
	return wi.root()+(booster::locale::format("/changes/{1}") % p).str();
}

void index::changes(std::string page_no)
{
	std::string key=locale_name+"_changes_"+page_no;
	if(cache().fetch_page(key))
		return;
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
	content::recent_changes c;	
	c.content.resize(rs.rows());
	row r;
	int n;
	for(n=0;rs.next(r);n++) {
		content::recent_changes::element &d=c.content[n];
		std::string lang,slug;
		std::tm created;
		r>>d.title>>d.version>>created>>d.author>>lang>>slug;
		d.created = mktime(&created);
		d.url=wi.page.page_version_url(d.version,lang,slug);
		if(d.version>1)
			d.diff_url=wi.page.diff_url(d.version-1,d.version,lang,slug);
	}
	if(c.content.size()==window)
		c.next=changes_url(p+1);
	ini(c);
	render("recent_changes",c);
	cache().store_page(key,60);
	// Cache changes for at most 30 sec
	// Generally -- prevent cache drop with frequent updates
}

void index::display_index()
{
	std::string key=locale_name+"_toc_index";
	if(cache().fetch_page(key))
		return;
	content::toc c;
	ini(c);
	result res;
	sql<<	"SELECT slug,title FROM pages "
		"WHERE lang=? "
		"ORDER BY title ASC",locale_name,res;
	unsigned items=res.rows();
	unsigned items_left=items/3;
	unsigned items_mid=items*2/3;
	std::string letter="";
	row r;
	for(unsigned i=0;res.next(r);i++) {
		std::vector<content::toc::element> *v;
		if(i<items_left)
			v=&c.left_col;
		else if(i<items_mid)
			v=&c.middle_col;
		else 
			v=&c.right_col;

		std::string t,slug;
		r>>slug>>t;
		if(!t.empty() && utf8::is_valid(t.begin(),t.end()))
		{
			std::string::iterator p=t.begin();
			utf8::next(p,t.end());
			std::string l(t.begin(),p);
			if(letter!=l) {
				content::toc::element e;
				e.letter=l;
				v->push_back(e);
				letter=l;
			}
			content::toc::element e;
			e.title=t;
			e.url=wi.page.page_url(locale_name,slug);
			v->push_back(e);
		}
	}
	render("toc",c);
	cache().store_page(key,30);
	// Cache TOC for at most 30 seconds
}
}
