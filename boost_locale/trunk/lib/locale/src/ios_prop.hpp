#ifndef BOOST_SRC_LOCALE_IOS_PROP_HPP
#define BOOST_SRC_LOCALE_IOS_PROP_HPP
#include <ios>

namespace boost {
    namespace locale {
        namespace impl {
        
            class ios_prop {
            public:
                static void set(Property const &prop,std::ios_base &ios)
                {
                    int id=get_id();
                    if(ios.pword(id)==0) {
                        ios.pword(id) = new Property(prop);
                        ios.register_callback(callback,id);
                    }
                    else if(ios.pword(id)==deleted) {
                        ios.pword(id) = new Property(prop);
                    }
                    else {
                        *reinterpret_cast<Property *>(ios.pword(id))=prop;
                    }
                }
                
                static Property &get(std::ios_base &ios)
                {
                    int id=get_id();
                    if(!has(ios))
                        set(Property(),ios);
                    return *reinterpret_cast<Property *>(ios.pword(id));
                }
                
                static bool has(std::ios_base &ios)
                {
                    int id=get_id();
                    if(ios.pword(id)==0 || ios.pword(id)==deleted)
                        return false;
                    return true;
                }

                static void unset(std::ios_base &ios)
                {
                    if(has(ios)) {
                        int id=get_id();
                        Property *p=reinterpret_cast<Property *>(ios.pword(id));
                        delete p;
                        ios.pword(id)=static_cast<void *>(-1);
                    }
                }
            private:
                static const void *deleted=static_cast<void *>(-1);
                
                static void callback(std::ios_base::event ev,std::ios_base &ios,int id)
                {
                    switch(ev) {
                    case std::ios_base::erase_event:
                        if(!has(ios))
                            break;
                        delete reinterpret_cast<Property *>(ios.pword(id));
                        break;
                    case std::ios_base::copyfmt_event:
                        if(ios.pword(id)==deleted || ios.pword(id)==0)
                            break;
                        ios.pword(id)=new Property(*reinterpret_cast<Property *>(ios.pword(id)));
                        break;
                    case std::ios_base::imbue_event:
                        if(ios.pword(id)==deleted || ios.pword(id)==0)
                            break;
                        reinterpret_cast<Property *>(ios.pword(id))->on_imbue(); 
                        break;
                        
                    default: ;
                    }
                }
                static int get_id()
                {
                    static const int id;
                    static boost::once_flag f=BOOST_ONCE_INIT;
                    boost::call_once(f,boost::bind(call_get_id,&id));
                    return id;
                }
                static void call_get_id(int *ptr)
                {
                    *ptr = std::ios_base::xalloc();
                }
            };




        }
    }
}



#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 

