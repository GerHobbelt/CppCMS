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
            class mo_file {
            public:
                typedef std::pair<char const *,char const *> pair_type;
                
                mo_file(std::string file_name) :
                    native_byteorder_(true),
                    size_(0)
                {
                    std::ifstream file(file_name.c_str(),std::ios::binary);
                    if(!file)
                        throw std::runtime_error("No such file");
                    // Check file format
                    uint32_t magic=0;
                    file.get(reinterpret_cast<char *>(&magic),sizeof(magic));
                    if(magic == 0x950412de)
                        native_byteorder_=true;
                    else if(magic == 0xde120495)
                        native_byteorder_=false;
                    else
                        throw std::runtime_error("Invalid file format");

                    // load image of file to memory
                    file.seekg(0, std::ios::end);
                    int len=file.tellg();
                    file.seekg(0, std::ios::beg);
                    data_.resize(len,0);
                    file.read(&data_.front(),len);
                    if(file.gcount()!=len)
                        throw std::runtime_error("Failed to read file");

                    // Read all format sizes
                    size_=get(8);
                    keys_offset_=get(12);
                    translations_offset_=get(16);
                    hash_offset_=get(20);
                    hash_size_=get(24);
                }

                pair_type find(char const *key_in) const
                {
                    uint32_t hkey = pj_winberger_hash_function(key_in);
                    uint32_t orig=hkey;
                    uint32_t incr = 1 + hkey % (hash_size_-2);
                    do {
                        uint32_t idx = get(hash_offset_ + 4*hkey);
                        /// Not found
                        if(idx == 0)
                            return pair_type(0,0);
                        /// If equal values return translation
                        if(strcmp(key(idx-1),key_in)==0)
                            return value(idx-1);
                        /// Rehash
                        hkey=(hkey + incr) % hash_size_;
                    } while(hkey!=orig)
                    return pair_type(0,0);                    
                }
                char const *key(int id) const
                {
                    uint32_t off = get(keys_offset_ + id*8 + 4);
                    return &data[off];
                }

                pair_type value(int id) const
                {
                    uint32_t len = get(translations_offset_+id*8);
                    uint32_t off = get(translations_offset_+id*8+4);
                    return pair_type(&data[off],&data[off]+len);
                }
                size_t size() const
                {
                    return size_;
                }
                bool empty()
                {
                    return size_==0;
                }
            private:
                uint32_t get(unsigned offset) const
                {
                    uint32_t tmp;
                    if(offset >= data.size() - 4)
                        throw std::runtime_error("Bad file format");
                    memcpy(&tmp,&data[offset],4);
                    convert(tmp);
                    return tmp;
                }

                void convert(uint32_t &v)
                {
                    if(native_byteorder_)
                        return;
                    v =   ((v & 0xFF) << 24)
                        | ((v & 0xFF00) << 8)
                        | ((v & 0xFF0000) >> 8)
                        | ((v & 0xFF000000) >> 24);
                }

                uint32_t keys_offset_;
                uint32_t translations_offset_;
                uint32_t hash_size_;
                uint32_t hash_offset_;

                std::vector<char> data_;
                bool native_byteorder_;
                size_t size_;
            };

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


                        //
                        // List of fallbacks: en_US@euro, en@euro, en_US, en. 
                        //
                        static const unsigned paths_no = 4;

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

                bool load_file(std::string file_name,std::string encoding,int id)
                {
                    try {
                        boost::shared_ptr<mo_file> mo(new mo_file(file_name));

                        std::string mo_encoding = extract(mo->key(0),"charset=");
                        if(sizeof(CharType) == 1 && ucnv_compareNames(mo_encoding.c_str(),encoding.c_str()) == 0) {
                            direct_catalogs_[id]=mo;
                        }
                        converter cvt(encoding,mo_encoding);
                        
                        for(unsigned i=0;i<mo.size();i++) {
                            std::pair<char const *,char const *> tmp= mo->value[i];
                            catalogs_[id][mo.key(i)]=cvt(tmp.first,tmp.second);
                        }

                    }
                    catch(std::exception const &err)
                    {
                        return false;
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
                std::vactor<boost::shared_ptr<mo_file> > mo_catalogs_;
                
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

