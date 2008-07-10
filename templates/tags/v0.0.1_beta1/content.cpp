#include "content.h"

namespace tmpl {

content::vector_t &content::vector(std::string const key,int reserve)
{
	vector_t &v=boost::any_cast<vector_t & >
		((*this)[key] = vector_t(reserve));
	return v;
}

content::list_t &content::list(std::string const key)
{
	return boost::any_cast<list_t & >((*this)[key] = list_t());
}

void	content::signal(std::string const &key,callback_t::slot_type slot)
{
	callback_ptr cb(new callback_t);
	cb->connect(slot);
	(*this)[key]=cb;
}

} // namespace tmpl


