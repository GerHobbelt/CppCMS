#include "all.h"
#include "textstream.h"


void Text_Stream::printf(char *format,...)
{
	va_list ap;
	
	va_start(ap,format);
	int size=vsnprintf(buffer,TS_BUFFER_SIZE,format,ap);
	va_end(ap);
	
	if(size>TS_BUFFER_SIZE) {
		char *str=new char[size+1];
		va_start(ap,format);
		vsnprintf(str,size+1,format,ap);
		va_end(ap);
		text += str;
		delete [] str;
	}
	else {
		text += buffer;
	}
}
