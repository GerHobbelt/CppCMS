#ifndef BOOST_SRC_LOCALE_MO_LAMBDA_HPP_INCLUDED
#define BOOST_SRC_LOCALE_MO_LAMBDA_HPP_INCLUDED

#include <memory>

namespace boost {
    namespace locale {
        namespace impl {
            namespace lambda {
                
                struct plural {

                    virtual int operator()(int n) const = 0;
                    virtual plural *clone() const = 0;
                    virtual ~plural()
                    {
                    }
                };

                typedef std::auto_ptr<plural> plural_ptr;

                plural_ptr compile(char const *c_expression);

            } // lambda 
        } // impl 
     } // locale 
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 

