#include "transtext.h"
#include <stdlib.h>
#include <cstring>

using namespace std;
namespace transtext {

static const trans default_trans;

trans const &trans_factory::get() const
{
	if(def.empty()) {
		return default_trans;
	}
	else {
		return get(def);
	}
}


trans const &trans_factory::get(string const &lang) const
{
	map<string,trans *>::const_iterator p;
	if((p=langs.find(lang))==langs.end()) {
		return default_trans;
	}
	else {
		return *(p->second);
	}
}

void trans_factory::load(string const &locale_list,string const &domain,string const &dir,string const &d)
{
	def=d;
	char *buffer=NULL;
	trans *tr_tmp=NULL;
	try{
		buffer=new char[locale_list.size()+1];
		strcpy(buffer,locale_list.c_str());
		char *locale,*ptr;
		langs["en"]=new trans;
		names["en"]="English";
		for(locale=strtok_r(buffer,",",&ptr);locale;locale=strtok_r(NULL,",",&ptr)) {
			if(langs.find(locale)!=langs.end())
				continue;
			tr_tmp=new trans_thread_safe();
			tr_tmp->load(locale,domain.c_str(),dir.c_str());
			string lname=tr_tmp->gettext("LANG");
			if(lname!="LANG") {
				langs[locale]=tr_tmp;
				tr_tmp=NULL;
				names[locale]=lname;
			}
			else {
				delete tr_tmp;
				tr_tmp=NULL;
			}
		}
	}
	catch(...) {
		delete [] buffer;
		delete tr_tmp;
		throw;
	}
	delete [] buffer;
}

trans_factory::~trans_factory()
{
	map<string,trans*>::iterator p;
	for(p=langs.begin();p!=langs.end();p++)
		delete p->second;
}


};
