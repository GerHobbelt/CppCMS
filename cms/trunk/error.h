#ifndef ERROR_H
#define ERROR_H

class Error {
	int code;
public:
	enum { E404,COMMENT_FIELDS, AUTH };
	Error(int x) :  code(x) {};
	int what() { return code ; };
};

#endif
