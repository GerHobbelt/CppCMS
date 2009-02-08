#ifndef DATA_H
#define DATA_H

#include <cppcms/application.h>
#include <cppcms/base_view.h>
#include <cppcms/form.h>
#include <string>



namespace data  {
using namespace cppcms;
using namespace std;

 
struct input_form : public form {
	widgets::number<int> arg;
	widgets::submit submit;
	input_form() :
		arg("arg","N"),
		submit("submit","Calc")
	{
		*this & arg & submit;
		arg.set_nonempty();
	}
};

struct message : public base_content {
	long long int fact;
	int arg;
	input_form info;
};
}


#endif
