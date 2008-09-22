#ifndef TMPL_CONTENT_H
#define TMPL_CONTENT_H

#include <map>
#include <list>
#include <string>
#include <vector>

#include <boost/any.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace tmpl {

class content : public std::map<std::string,boost::any> {
public:
	typedef std::list<content> list_t;
	typedef std::vector<content> vector_t;
	typedef boost::function<bool (content &)> callback_t;

	vector_t &vector(std::string const key,int reserve=0);
	list_t &list(std::string const key);
	void signal(std::string const &key,callback_t slot);

};

}; // namespace tmpl
#endif


