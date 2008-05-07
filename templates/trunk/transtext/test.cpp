#include "transtext.h"
#include <stdio.h>
#include <cstdlib>

using namespace transtext;


int main(int argc,char **argv)
{
	trans_factory tf;
	tf.load("en,he","test","./locale");

	int i;
	for(i=0;i<15;i++) {
		printf(tf["he"].ngettext("passed one day","passed %d days",i),i);
		putchar('\n');
		printf(tf["en"].ngettext("passed one day","passed %d days",i),i);
		putchar('\n');
	}

	return 0;
}
