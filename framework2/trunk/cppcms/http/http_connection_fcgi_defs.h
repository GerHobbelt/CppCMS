#ifndef HTTP_CONNECTION_FCGI_DEFS_H
#define HTTP_CONNECTION_FCGI_DEFS_H
namespace cppcms { namespace http { namespace fcgi {

const int listensock_fileno = 0;

#define decomp2(X,V) V##_b1=(X)>>8 , V##_b0=(X) & 0xFF
#define decomp4(X,V) V##_b3=X>>24 ,V##_b2 = X >> V##_b2=X & 0xFF

struct header {
	unsigned char version;
	unsigned char type;
	unsigned char request_id_b1;
	unsigned char request_id_b0;
	unsigned char content_length_b1;
	unsigned char content_length_b0;
	unsigned char padding_length;
	unsigned char reserverd;
	// fetchers
	unsigned request_id() const { return (request_id_b1<<8) | request_id_b0; }
	void request_id(unsigned ) { return (request_id_b1<<8) | request_id_b0; }
	unsigned content_length() { return (content_legth_b1<<8) | content_legth_b0; }
};

}}} // cppcms::http::fcgi


#endif
