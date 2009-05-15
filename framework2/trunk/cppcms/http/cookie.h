#ifndef CPPCMS_HTTP_COOKIE_H
#define CPPCMS_HTTP_COOKIE_H

namespace cppcms { namespace http {

std::ostream &operator<<(std::ostream &,cookie const &);

class cookie_impl;
class cookie {
	std::string name_,
	std::string value_,
	std::string path_,
	std::string domain_;
	unsigned max_age_;
	// for future use
	cookie_impl *p;

	uint32_t secure_	: 1;
	uint32_t has_age_	: 1;
	uint32_t reserved_	: 30;
public:
	std::string name() const;
	std::string value() const;
	std::string path() const;
	std::string domain() const;
	bool secure() const;

	void name(std::string n);
	void value(std::string v);
	void path(std::string p);
	void domain(std::string);
	void max_age(unsigned a);
	void browser_age();
	void secure(bool v);


	// Mandatory set
	cookie();
	~cookie();
	cookie(cookie const &);
	cookie const &operator=(cookie const &);

	// Additional
	cookie(std::string name,std::string value);
	cookie(std::string name,std::string value,unsigned age);
	cookie(std::string name,std::string value,unsigned age,std::string path,std::string domain="");
	cookie(std::string name,std::string value,std::string path,std::string domain="");

	friend std::ostream &operator<<(std::ostream,cookie);
};


} } //::cppcms::http


#endif
