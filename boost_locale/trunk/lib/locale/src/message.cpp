namespace boost {
    namespace locale {

        struct messages_loader::data {}; // nothing
        messages_loader::messages_loader()
        {
        }
        messages_loader::~messages_loader()
        {
        }

        void messages_loader::add_domain(std::string dm)
        {
            if(domains_.empty()) {
                default_domain_ = dm;
            }
            domains_.insert(domain);
        }

        void messages_loader::add_path(std::string path)
        {
            paths_.push_back(path);
        }
        void messages_loader::domain(std::string dm)
        {
            add_domain(dm);
            default_domain_ = dm;
        }

        namespace impl {

            template<typename CharType>
            class mo_message : public message_format<CharType> {
            public:

                virtual CharType const *get_string(int domain_id,char const *id) const
                {
                    string_type const *str_ptr = get_string(domain_id,id);
                    if(!str_ptr)
                        return 0;
                    return str_ptr->c_str();
                }
                virtual CharType const *get(int domain_id,char const *single_id,int n) const
                {
                    string_type const *str_ptr = get_string(domain_id,id);
                    if(!str_ptr)
                        return 0;
                    int form=0;
                    if(plural_forms_.at(domain_id)) 
                        form = plural_forms_[domain_id](n);
                    else
                        form = n == 1 ? 0 : 1; // Fallback to english plural form

                    size_t p=0;
                    for(int i=0;i<form;i++) {
                        size_t p=str_ptr->find(0,p);
                        if(p==string_type::npos)
                            return 0;
                        ++p;
                    }
                    if(p >= str_ptr->size())
                        return 0;
                    return str_ptr->c_str() + p;
                }

                virtual int domain(std::string const &domain) const
                {
                    domains_map_type::const_iterator p=domains_.find(domain);
                    if(p==domains_.end())
                        return -1;
                    return p->second;
                }

                void add_path(std::string path)
                {
                    search_paths_.push_back(path);
                }

                void add_domain(std::string domain)
                {
                    if(domains_.find(domain)==domains_.end())
                        return;

                    catalogs_.resize(catalogs_.size() + 1);
                    plural_forms_.resize(plural_forms_.size() + 1);

                    domains_[domain]=catalogs_.size() - 1;
                }

                void set_default_domain(std::string domain)
                {
                   add_domain(domain); 
                   for(domains_::iterator p=domains_.begin(),e=domains_.end();++p) {
                       if(p->second==0)
                           break;
                   }
                   std::string swap_domain = p->first;
                   int id=domains_[domain];
                   domains_[swap_domain]=id;
                   domains_[domain]=0;

                }

                void load(std::locale const &loc)
                {
                    info const &inf = std::use_faset<info>(loc);
                    for(domain_::iterator p=domain_.begin(),e=domain_.end();++p) {
                        std::string domain=p->first;
                        int id=p->second;

                        static const unsigned paths_no = 4;

                        /// List of fallbacks
                        std::string paths[paths_no] = {
                            std::string(inf.language()) + "_" + inf.country() + "@" + inf.variant(),
                            std::string(inf.language()) + "@" + inf.variant(),
                            std::string(inf.language()) + "_" + inf.country(),
                            std::string(inf.language()),
                        };
                        
                        for(unsigned j=0;j<paths_no;j++) {
                            for(unsigned i=0;i<search_paths_.size();i++) {

                                std::string full_path = search_paths_[i]+"/"+paths[j]+"/LC_MESSAGES/"+domain+".mo";

                                if(load_file(full_path,catalogs_[id])) {
                                    plural_forms_[id]=get_plural_form(catalogs_[id]);
                                    break;
                                }
                                else {
                                    catalogs_[id].clear();
                                }
                            }
                        }

                    }
                }

            private:
                std::auto_ptr<lambda::plural> get_plural_form(catalog_type const &catalog)
                {
                    std::auto_ptr<lambda::plural> form=lambda::compile(extract(catalog[""],"plural="));
                    return form;
                }

                bool load_file(std::string file_name,std::string encoding,catalog_type &catalog)
                {
                    std::ifstream file(file_name.c_str(),std::ifstream::binary);
                    if(!file)
                        return false;
                    mo_file mo(file);
                    if(mo.empty())
                        return false;
                    cstd::string mo_encoding = extract(mo.key(0),"charset=");

                    converter cvt(encoding,mo_encoding);
                    
                    for(unsigned i=0;i<mo.size();i++) {
                        std::pair<char const *,char const *> tmp= mo.value[i];
                        catalog[mo.key(i)]=cvt(tmp.first,tmp.second);
                    }

                }

                static std::string extract(std::string meta,std::string key)
                {
                    size_t pos=meta.find(key);
                    if(pos == std::string::npos)
                        return "";
                    pos+=key.size(); /// size of charset=
                    static const std::string separator="\t\r\n\ ;";
                    size_t end_pos = meta.find_first_of(separator,pos);
                    return meta.substr(pos,end_pos - pos);
                }

                class mo_file {
                public:
                    typedef std::pair<char const *,char const *> pair_type;
                    
                    mo_file(std::ifsteam &file)
                    
                    char const *key(int id)
                    {
                    }
                    pair_type value(int id)
                    {
                    }
                    size_t size()
                    {
                        return size_;
                    }
                    bool empty()
                    {
                        return size==0;
                    }


                private:
                    size_t size_;
                };

                class converter {
                public:
                    converter(std::string out_enc,in_enc) :
                        out_(out_enc),
                        in_(in_enc),
                    {
                    }

                    std::basic_string<CharType> operator()(char const *begin,char const *end)
                    {
                        return out_.std(in_.icu(begin,end));
                    }

                private:
                    icu_std_converter<CharType> out_;
                    icu_std_converter<char> in_;
                };


            protected:

                typedef std::basic_string<CharType> string_type;
                typedef boost::unordered_map<std::string,string_type> catalog_type;
                typedef std::vector<catalog_type> catalogs_set_type;
                typedef std::map<std::string,int id> domains_map_type;

                virtual string_type const *get_string(int domain_id,char const *id) const
                {
                    if(domian_id < 0 || domain_id >= catalogs_.size())
                        return 0;
                    catalog_type const &cat = catalogs_[domain_id];
                    catalog_type::const_iterator p = cat.find(id);
                    if(p==cat.end())
                        return 0;
                    return &p->second;
                }

                catalogs_set_type catalogs_;
                std::vector<boost::shared_ptr<lambda::plural> > plural_forms_;
                domains_map_type domains_;

                std::vector<std::string> search_paths_;
                
            };
        } /// impl


        template<typename CharType>
        message_format<CharType> *message_loader::generate(std::locale const &loc)
        {
            std::auto_ptr<impl::mo_message<CharType> > ptr(new impl::mo_message)
            for(unsigned i=0;i<paths_.size();i++)
                ptr->add_path(paths_[i]);
            std::set<std::string>::const_iterator p;
            for(p=domains_.begin();p!=domains_.end();++p) {
                ptr->add_domain(*p);
            }
            ptr->set_default_domain(default_domain_);
            ptr->load(loc);
            return ptr.release();
            
        }

        std::locale message_loader::load(std::locale const &base,facet_type facets)
        {
            std::locale res = base;
            if(facets & char_facet)
                res=std::locale(res,generate<char>(base));
            #ifndef BOOST_NO_STD_WSTRING
            if(facets & wchar_t_facet) 
                res=std::locale(res,generate<wchar_t>(base));
            #endif
            #ifdef BOOST_HAS_CHAR16_T
            if(facets & char16_t_facet) 
                res=std::locale(res,generate<char16_t>(base));
            #endif
            #ifdef BOOST_HAS_CHAR32_T
            if(facets & char32_t_facet) 
                res=std::locale(res,generate<char32_t>(base));
            #endif
            return res;
        }
    }
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

