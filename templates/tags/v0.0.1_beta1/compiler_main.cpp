#include "Parser.h"

#include <iostream>

int Parser::lex()
{
	std::string s;
	if(std::cin.eof()) return 0;
	std::cin>>s;
	if(s=="inline") {
		d_val=boost::any(std::string("Hello World"));
		return INLINE;
	}
	else if(s=="if") {
		return IF;
	}
	else if(s=="else") return ELSE;
	else if(s=="template") return TEMPLATE;
	else if(s=="end") return END;
	else if(s=="elseif") return ELSEIF;
	else if(s=="foreach") return FOREACH;
	else if(s=="in") return IN;
	else if(s=="item") return ITEM;
	else if(s=="empty") return EMPTY;
	else if(s=="include") return INCLUDE;
	else if(s=="not") return NOT;
	else if(s=="using") return USING;
	else if(s==".") return '.';
	else {
		d_val=boost::any(s);
		return IDENT;
	}
}

int main()
{
	Parser p;
	p.parse();
	return 0;
}
