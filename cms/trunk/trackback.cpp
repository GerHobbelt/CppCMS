#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include "trackback.h"

using namespace std;

namespace details {
	class curl_init{
	public:
		curl_init() {curl_global_init(CURL_GLOBAL_ALL);};
	} init;
};

static string post_message(string name,string value)
{
	char *d=curl_escape(value.c_str(),value.size());
	if(!d) return "";
	string tmp=name+"="+d;
	curl_free(d);
	return tmp;
}

static size_t write_function(void *ptr, size_t size, size_t nmemb, void *stream)
{
	if(stream){
		((string *)(stream))->append((char *)ptr,size*nmemb);
		return size*nmemb;
	}
	return 0;
}

static int post_data(string target,string post_data,string charset,string &result)
{
  CURL *curl;
  CURLcode res=CURLE_OK;

  string buf="Content-Type: application/x-www-form-urlencoded; charset="+charset;
  static const char expect[]="Expect:";

  curl_slist *headerlist = curl_slist_append(NULL, buf.c_str());
  headerlist=curl_slist_append(headerlist,expect);
  curl = curl_easy_init();
  if(curl) {
    /* what URL that receives this POST */
    curl_easy_setopt(curl, CURLOPT_URL, target.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,write_function);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,&result);
    res = curl_easy_perform(curl);


    /* always cleanup */
    curl_easy_cleanup(curl);

    /* free slist */
    curl_slist_free_all (headerlist);
  }
  if(res!=CURLE_OK){
  	result=curl_easy_strerror(res);
	return 1;
  }
  return 0;
}

void trackback::set_data(string n,string v)
{
	if(post_data=="")
		post_data=post_message(n,v);
	else
		post_data+="&"+post_message(n,v);
}

bool trackback::post(string &e)
{
	string result;
	if(::post_data(target,post_data,charset,result)!=0) {
		e=result;
		return false;
	}
	if(result.find("<error>0</error>")!=string::npos){
		return true;
	}
	size_t p1,p2;
	p1=result.find("<message>");
	p2=result.find("</message>");
	if(p1!=string::npos && p2!=string::npos && p1+9<p2) {
		e=result.substr(p1+9,p2-(p1+9));
	}
	else {
		e="Unreadable server respond";
	}
	return false;
}

#ifdef TESTME

int main()
{
	trackback t("http://localhost:8080/blog/trackback/11","utf-8");
	t.url("www.google.com");
	t.blog_name("Blog");
	t.title("My Blog");
	string m;
	if(t.post(m))
		cout<<"OK\n";
	else
		cout<<"Error:"<<m<<endl;
}

#endif 
