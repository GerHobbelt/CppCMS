#if defined __linux || defined __CYGWIN__
#  define HAS_FOPENCOOKIE
#  define HAS_OPEN_MEMSTREAM
#elif defined(__FreeBSD__) || defined(__APPLE__)
#  define HAS_FWOPEN
#endif

#if defined HAS_FOPENCOOKIE || defined HAS_OPEN_MEMSTREAM
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <stdexcept>
#include "markdown.h"

extern "C" {
	#include <mkdio.h>
}


extern "C" ssize_t fopencookie_write(void *cp,char const *buf,size_t size)
{
	try {
		reinterpret_cast<std::string *>(cp)->append(buf,size);
	}
	catch(std::bad_alloc const &e) { 
		return -1; 
	}
	return size;
}

extern "C" int fwopen_write(void *cp,char const *buf,int size)
{
	try {
		reinterpret_cast<std::string *>(cp)->append(buf,size);
	}
	catch(std::bad_alloc const &e) {
		return -1; 
	}
	return size;
}


FILE *fopen_std_string(std::string &output)
{
	#if defined HAS_FOPENCOOKIE
	cookie_io_functions_t io = { 0 , fopencookie_write, 0 , 0 };
	return fopencookie(reinterpret_cast<void *>(&output),"w",io);
	#elif defined HAS_FWOPEN
	return fwopen(reinterpret_cast<void *>(&output),fwopen_write);
	#else
	return 0;
	#endif
}


std::string markdown_to_html(char const *str,int len,int flags)
{
	/// It is safe to const cast as mkd_string does not 
	/// alter original string
	MMIOT *doc = mkd_string(const_cast<char *>(str),len,flags);
	if(!doc) {
		throw std::runtime_error("Failed to read document");
	}

	mkd_compile(doc,flags);
	
	std::string result;

	#if defined HAS_FOPENCOOKIE || defined HAS_FWOPEN
		FILE *output = fopen_std_string(result);
	#elif defined HAS_OPEN_MEMSTREAM
		char *buf=0;
		size_t buf_len = 0;
		FILE *output = open_memstream(&buf,&buf_len);
	#else
		FILE *output = tmpfile();
	#endif

	if(!output) {
		mkd_cleanup(doc);
		throw std::runtime_error("Failed to create output stream");
	}

	mkd_generatetoc(doc,output);
	mkd_generatehtml(doc,output);
	mkd_cleanup(doc);
	
	#if defined HAS_FOPENCOOKIE || defined HAS_FWOPEN
		fclose(output);
	#elif defined(HAS_OPEN_MEMSTREAM)
		fclose(output);
		result.assign(buf,buf_len);
		free(buf);
	#else // Fall back to temporary file
		fseek(output,0,SEEK_END);
		long size = ftell(output);
		std::vector<char> v(size+1,0);
		size = fread(&v,1,size,output);
		v.resize(size+1,0);
		result = &v.front();
		fclose(output);
	#endif

	return result;
}


std::string markdown_format_for_highlighting(std::string const &input,std::string const &html_class)
{
	enum { part_a , part_b } state = part_a;
	std::string repla = "<pre name=\"code\" class=\"" + html_class + "\">";
	std::string replb = "</pre>";
	std::string origa="<pre><code>";
	std::string origb="</code></pre>";
	std::string result;
	result.reserve(input.size());
	size_t pos = 0;
	while(pos < input.size()) {
		std::string const &orig = state == part_a ? origa : origb;
		std::string const &repl = state == part_a ? repla : replb;
		size_t next = input.find(orig,pos);
		if(next == std::string::npos)
			next = input.size();
		result.append(input.data() + pos, next - pos);
		if(next < input.size()) {
			result.append(repl);
			pos = next + orig.size();
			if(state == part_a)
				state = part_b;
			else
				state = part_a;
		}
		else {
			pos = next;
		}
	}
	return result;
}
