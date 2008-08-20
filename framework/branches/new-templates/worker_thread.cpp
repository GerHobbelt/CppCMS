#include "worker_thread.h"
#include "global_config.h"
#include "thread_cache.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "manager.h"

using namespace cgicc;
namespace cppcms {

worker_thread(manager const &s) :
		url(this),
		app(s),
		cache(this),
		cout(&(this->out_buf))
{
	caching_module=app.cache->get();
} ;

worker_thread::~worker_thread()
{
	app.cache->del(caching_module);
}
void worker_thread::main()
{
	out="<h1>Hello World</h2>\n";
}
void worker_thread::set_header(HTTPHeader *h)
{
	response_header=auto_ptr<HTTPHeader>(h);
};
void worker_thread::add_header(string s) { 
	other_headers.push_back(s); 
};

void worker_thread::set_lang()
{
	gt=&app.gettext->get();
}
void worker_thread::set_lang(string const &s)
{
	gt=&app.gettext->get(s);
}

void worker_thread::run(cgicc_connection &cgi_conn)
{
	cgi=&cgi_conn.cgi();
	env=&(cgi->getEnvironment());
	ostream &cgi_out=cgi_conn.cout();
	set_lang();
	other_headers.clear();
	cache.reset();
	out_buf.str("");

	set_header(new HTTPHTMLHeader);

	gzip=gzip_done=false;
	string encoding;

	if((encoding=cgi_conn.env("HTTP_ACCEPT_ENCODING"))!="") {
		if(strstr(encoding.c_str(),"gzip")!=NULL) {
			gzip=app.config.lval("gzip.enable",0);
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
	catch(std::exception const &e) {
		string msg=e.what();
		set_header(new HTTPStatusHeader(500,msg));
		cgi_out<<"<html><body><p>"+msg+"</p><body></html>";
		gzip=gzip_done=false;
		other_headers.clear();
		out_buf.str("");
		return;
	}

	if(app.config.lval("server.disable_xpowered_by",0)==0) {
		add_header("X-Powered-By: CppCMS/0.0alpha1");
	}

	for(list<string>::iterator h=other_headers.begin();h!=other_headers.end();h++) {
		cgi_out<<*h<<"\n";
	}

	string out=out_buf.str();
	out_buf.str("");
	
	if(gzip) {
		if(out.size()>0) {
			if(gzip_done){
				cgi_out<<"Content-Length: "<<out.size()<<"\n";
			}
			cgi_out<<"Content-Encoding: gzip\n";
			cgi_out<<*response_header;
			if(gzip_done) {
				cgi_out<<out;
			}
			else{
				long level=app.config.lval("gzip.level",-1);
				long length=app.config.lval("gzip.buffer",-1);
				deflate(out,cgi_out,level,length);
			}
		}
		else {
			cgi_out<<*response_header;
		}
	}
	else {
		cgi_out<<"Content-Length: "<<out.size()<<"\n";
		cgi_out<<*response_header;
		cgi_out<<out;
	}
}



}



