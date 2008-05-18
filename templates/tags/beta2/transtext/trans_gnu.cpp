#include "transtext.h"
#include <locale.h>
#include <libintl.h>

namespace transtext {

void trans_gnu::load(char const * locale,char const *domain_name, char const * dirname)
{
	setlocale(LC_ALL,locale);
	bindtextdomain(domain_name,dirname);
	textdomain(domain_name);
};

char const *trans_gnu::gettext(char const *s) const 
{
	return ::gettext(s);
} 

char const *trans_gnu::ngettext(char const *s,char const *p,int n) const 
{
	return ::ngettext(s,p,n);
}

};
