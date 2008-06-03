#include "worker_thread.h"
#include "global_config.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

using namespace cgicc;
namespace cppcms {

void worker_thread::main()
{
	out="<h1>Hello World</h2>\n";
}

static void deflate(FCgiIO &io,string const &out)
{
	using namespace boost::iostreams;
	gzip_params params;
	long level,length;

	if((level=global_config.lval("gzip.level",-1))!=-1){
		params.level=level;
	}		

	filtering_ostream zstream;

	if((length=global_config.lval("gzip.buffer",-1))!=-1){
		zstream.push(gzip_compressor(params,length));
	}
	else {
		zstream.push(gzip_compressor(params));
	}

	zstream.push(io);
	zstream<<out;
}

void worker_thread::run(FCGX_Request *fcgi)
{
	io = auto_ptr<FCgiIO>(new FCgiIO(*fcgi));
	cgi = auto_ptr<Cgicc>(new Cgicc(&*io));
	env=&(cgi->getEnvironment());

	out.clear();
	out.reserve(global_config.lval("performance.textalloc",65500));
	cache.reset();

	set_header(new HTTPHTMLHeader);

	gzip=gzip_done=false;
	char *ptr;
	if((ptr=FCGX_GetParam("HTTP_ACCEPT_ENCODING",fcgi->envp))!=NULL) {
		if(strstr(ptr,"gzip")!=NULL) {
			gzip=global_config.lval("gzip.enable",0);
		}
	}

	try {
		/**********/
		main();
		/**********/
		if(response_header.get() == NULL) {
			throw cppcms_error("Looks like a bug");
		}
	}
	catch(cppcms_error const &e) {
		string msg=e.what();
		set_header(new HTTPStatusHeader(500,msg));
		out="<html><body><p>"+msg+"</p><body></html>";
	}


	if(gzip) {
		*io<<"Content-Encoding: gzip\r\n";
		*io<<*response_header;
		if(gzip_done)
			*io<<out;
		else
			deflate(*io,out);
	}	
	else {
		*io<<*response_header;
		*io<<out;
	}

	// Clean Up
	out.clear();
	response_header.reset();
	cgi.reset();
	io.reset();
        FCGX_Finish_r(fcgi);
}

}
