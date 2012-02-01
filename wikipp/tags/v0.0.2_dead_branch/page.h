#ifndef PAGE_H
#define PAGE_H

#include "master.h"
#include <cppcms/url.h>
#include <string>
using namespace cppcms;
using namespace std;

namespace data { 
	class page;
	class edit_page;
	class page_form;
}

namespace apps {

class page : public master {
	void save(int id,data::page_form &);
	bool load(data::page_form &);
	bool edit_on_post(data::edit_page &);
	bool load_history(int,data::page_form &);
protected:
	string slug;
	void ini(data::page &);
	void display(string slug);
	void history(string slug,string page);
	void display_ver(string slug,string sid);
	void edit(string slug,string version);
	void diff(string slug,string v1,string v2);

	string edit_url();
	string edit_version_url(int ver);
	string history_url(int n=0);
public:
	page(wiki &w);
	string diff_url(int v1,int v2,string lang="",string s="");
	string page_url(string l="",string s="");
	string page_version_url(int ver,string l="",string s="");
	string default_page_url(string l="en",string s="main")
		{ return page_url(l,s); }
	void redirect(string locale="en",string slug="main");
};

}


#endif
