#include "transtext.h"
#include <stdio.h>
#include <cstdlib>

using namespace transtext;


int main(int argc,char **argv)
{
	trans_thread_safe t1;
	trans_gnu t2;

	t1.load("he","test","./locale");
	t2.load("he_IL.UTF-8","test","./locale");

	int i;
	for(i=0;i<15;i++) {
		printf(t1.ngettext("passed one day\n","passed %d days\n",i),i);
		printf(t2.ngettext("passed one day\n","passed %d days\n",i),i);
	}

	return 0;
}
