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
	widgets::select matrial;
	widgets::number<double> age;
	widgets::submit submit;
	info_form() :
		name("name","Your Name"),
		sex("sex","Sex"),
		matrial("mat","Matrial State"),
		age("age","Your Age"),
		submit("submit","Send")
	{
		*this & name & sex & matrial & age & submit;
		sex.add("Male");
		sex.add("Female");
		matrial.add("Single");
		matrial.add("Married");
		matrial.add("Divorced");
		name.set_nonempty();
		age.set_range(0,120);
	}
	virtual bool validate()
	{
		if(!form::validate()) return false;
		if(matrial.get()!="Single" && age.get()<18) {
			matrial.is_valid=false;
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
