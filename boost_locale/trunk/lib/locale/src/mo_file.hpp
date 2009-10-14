#include "mo_lambda.hpp"

namespace boost {
    namespace locale {
        namespace impl {

            class mo_file : public boost::noncopyable
            {
            public:

                ///
                /// Load catalog to memory.
                /// Throws std::runtime_error if load fails
                ///
                mo_file(std::string file_name); 
                ~mo_file()

                ///
                /// Return number of strings in catalog
                ///
                unsigned strings() const;

                ///
                /// Return native I/O charset of the catalog
                ///
                char const *charset() const;

                ///
                /// Lookups string by id and offset(for plural form)
                ///
                char const *get_string(char const *id,unsigned plural_form_id = 0) const;
                
                ///
                /// Returns plural form id for number
                unsigned pluralize(int n) const;
            private:
                std::unordered_map<std::string,std::string> map_; 
            }



        } // impl
    } // locale 
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
