#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <stdexcept>
#include "markdown.h"

extern "C" {
	#include <mkdio.h>
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
	result.reserve(len);
	char *content_ptr = 0;
	int content_size = 0;
	char *toc_ptr = 0;
	int toc_size = 0;

	content_size = mkd_document(doc,&content_ptr);
	if(flags & mkd::toc) {
		toc_size = mkd_toc(doc,&toc_ptr);
		result.assign(toc_ptr,toc_size);
	}
	result.append(content_ptr,content_size);
	free(toc_ptr);
	mkd_cleanup(doc);

	return result;
}


