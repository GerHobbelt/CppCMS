#ifndef DATA_H
#define DATA_H

#include <cppcms/application.h>
#include <cppcms/base_view.h>
#include <cppcms/form.h>
#include <string>



namespace data  {
using namespace cppcms;
using namespace std;

 
struct info_form : public form {
	widgets::text name;
	widgets::radio sex;
	widgets::select martial;
	widgets::number<double> age;
	widgets::submit submit;
	info_form() :
		name("name","Your Name"),
		sex("sex","Sex"),
		martial("mat","martial State"),
		age("age","Your Age"),
		submit("submit","Send")
	{
		*this & name & sex & martial & age & submit;
		sex.add("Male");
		sex.add("Female");
		martial.add("Single");
		martial.add("Married");
		martial.add("Divorced");
		name.set_nonempty();
		age.set_range(0,120);
	}
	virtual bool validate()
	{
		if(!form::validate()) return false;
		if(martial.get()!="Single" && age.get()<18) {
			martial.not_valid();
			return false;
		}
		return true;
	}
};

struct message : public base_content {
	string name,state,sex;
	double age;
	info_form info;
};
}


#endif
