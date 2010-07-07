//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_LOCALE_SOURCE
#include <boost/config.hpp>
#include <boost/locale/info.hpp>
#include <boost/locale/message.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/locale/codepage.hpp>
#if BOOST_VERSION >= 103600
#include <boost/unordered_map.hpp>
#else
#include <map>
#endif


#include "message.hpp"
#include "mo_hash.hpp"
#include "mo_lambda.hpp"

#include <stdio.h>
#include <iostream>

#include <string.h>

namespace boost {
    namespace locale {
        namespace impl {
            class mo_file {
            public:
                typedef std::pair<char const *,char const *> pair_type;
                
                mo_file(std::string file_name) :
                    native_byteorder_(true),
                    size_(0)
                {
                    load_file(file_name);
                    // Read all format sizes
                    size_=get(8);
                    keys_offset_=get(12);
                    translations_offset_=get(16);
                    hash_size_=get(20);
                    hash_offset_=get(24);
                }

                pair_type find(char const *key_in) const
                {
                    pair_type null_pair((char const *)0,(char const *)0);
                    if(hash_size_==0)
                        return null_pair;
                    uint32_t hkey = pj_winberger_hash_function(key_in);
                    uint32_t incr = 1 + hkey % (hash_size_-2);
                    hkey %= hash_size_;
                    uint32_t orig=hkey;
                    
                    
                    do {
                        uint32_t idx = get(hash_offset_ + 4*hkey);
                        /// Not found
                        if(idx == 0)
                            return pair_type((char const *)0,(char const *)0);
                        /// If equal values return translation
                        if(strcmp(key(idx-1),key_in)==0)
                            return value(idx-1);
                        /// Rehash
                        hkey=(hkey + incr) % hash_size_;
                    } while(hkey!=orig);
                    return null_pair;
                }
                
                char const *key(int id) const
                {
                    uint32_t off = get(keys_offset_ + id*8 + 4);
                    return data_ + off;
                }

                pair_type value(int id) const
                {
                    uint32_t len = get(translations_offset_ + id*8);
                    uint32_t off = get(translations_offset_ + id*8 + 4);
                    return pair_type(&data_[off],&data_[off]+len);
                }

                bool has_hash() const
                {
                    return hash_size_ != 0;
                }

                size_t size() const
                {
                    return size_;
                }

                bool empty()
                {
                    return size_ == 0;
                }

            private:
                void load_file_direct(std::string file_name)
                {
                    FILE *file = 0;
                    #if defined(BOOST_WINDOWS)
                        /// Under windows we have to use "_wfopen" to get
                        /// access to path's with Unicode in them
                        ///
                        /// As not all standard C++ libraries support nonstandard std::istream::open(wchar_t const *)
                        /// we would use old and good stdio and _wfopen CRTL functions
                        ///

                        if(file_name.compare(0,3,"\xEF\xBB\xBF")==0)
                        {
                            std::wstring wfile_name = conv::to_utf<wchar_t>(file_name.substr(3),"UTF-8");
                            file = _wfopen(wfile_name.c_str(),L"rb");
                        }
                        else {
                            file = fopen(file_name.c_str(),"rb");
                        }

                    #else // POSIX systems do not have all this Wide API crap

                        file = fopen(file_name.c_str(),"rb");

                    #endif
                    if(!file)
                        throw std::runtime_error("No such file");

                    try {
                        uint32_t magic=0;
                        fread(&magic,4,1,file);
                        
                        if(magic == 0x950412de)
                            native_byteorder_ = true;
                        else if(magic == 0xde120495)
                            native_byteorder_ = false;
                        else
                            throw std::runtime_error("Invalid file format");
                        
                        fseek(file,0,SEEK_END);
                        long len=ftell(file);
                        if(len < 0) {
                            throw std::runtime_error("Wrong file object");
                        }
                        fseek(file,0,SEEK_SET);
                        vdata_.resize(len+1,0); // +1 to make sure the vector is not empty
                        if(fread(&vdata_.front(),1,len,file)!=unsigned(len))
                            throw std::runtime_error("Failed to read file");
                        data_ = &vdata_[0];
                        file_size_ = len;
                    }
                    catch(...) {
                        if(file)
                            fclose(file);
                        throw;
                    }
                    fclose(file);
                }
                
                void load_file(std::string file_name)
                {
                   load_file_direct(file_name);
                }

                uint32_t get(unsigned offset) const
                {
                    uint32_t tmp;
                    if(offset > file_size_ - 4) {
                        throw std::runtime_error("Bad file format");
                    }
                    memcpy(&tmp,data_ + offset,4);
                    convert(tmp);
                    return tmp;
                }

                void convert(uint32_t &v) const
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

                char const *data_;
                size_t file_size_;
                std::vector<char> vdata_;
                bool native_byteorder_;
                size_t size_;
            };

            template<typename CharType>
            class mo_message : public message_format<CharType> {

                typedef std::basic_string<CharType> string_type;
                #if BOOST_VERSION >= 103600
                typedef boost::unordered_map<std::string,string_type> catalog_type;
                #else
                typedef std::map<std::string,string_type> catalog_type;
                #endif
                typedef std::vector<catalog_type> catalogs_set_type;
                typedef std::map<std::string,int> domains_map_type;
            public:

                typedef std::pair<CharType const *,CharType const *> pair_type;

                virtual CharType const *get(int domain_id,char const *context,char const *id) const
                {
                    return get_string(domain_id,context,id).first;
                }

                virtual CharType const *get(int domain_id,char const *context,char const *single_id,int n) const
                {
                    pair_type ptr = get_string(domain_id,context,single_id);
                    if(!ptr.first)
                        return 0;
                    int form=0;
                    if(plural_forms_.at(domain_id)) 
                        form = (*plural_forms_[domain_id])(n);
                    else
                        form = n == 1 ? 0 : 1; // Fallback to english plural form

                    CharType const *p=ptr.first;
                    for(int i=0;p < ptr.second && i<form;i++) {
                        p=std::find(p,ptr.second,0);
                        if(p==ptr.second)
                            return 0;
                        ++p;
                    }
                    if(p>=ptr.second)
                        return 0;
                    return p;
                }

                virtual int domain(std::string const &domain) const
                {
                    domains_map_type::const_iterator p=domains_.find(domain);
                    if(p==domains_.end())
                        return -1;
                    return p->second;
                }

                mo_message(messages_info<CharType> &inf)
                {
                    catalogs_.resize(inf.domains.size());
                    mo_catalogs_.resize(inf.domains.size());
                    plural_forms_.resize(inf.domains.size());

                    for(unsigned id=0;id<inf.domains.size();id++) {
                        std::string domain=inf.domains[id];
                        domains_[domain]=id;
                        //
                        // List of fallbacks: en_US@euro, en@euro, en_US, en. 
                        //
                        static const unsigned paths_no = 4;

                        std::string paths[paths_no] = {
                            std::string(inf.language) + "_" + inf.country + "@" + inf.variant,
                            std::string(inf.language) + "@" + inf.variant,
                            std::string(inf.language) + "_" + inf.country,
                            std::string(inf.language),
                        };

                        bool found=false; 
                        for(unsigned j=0;!found && j<paths_no;j++) {
                            for(unsigned i=0;!found && i<inf.paths.size();i++) {
                                std::string full_path = inf.paths[i]+"/"+paths[j]+"/LC_MESSAGES/"+domain+".mo";

                                found = load_file(full_path,inf,id);
                            }
                        }
                    }
                }

                virtual ~mo_message()
                {
                }

            private:

                bool load_file(std::string file_name,messages_info<CharType> &inf,int id)
                {
                    try {
                        std::auto_ptr<mo_file> mo(new mo_file(file_name));

                        std::string plural = extract(mo->value(0).first,"plural=","\r\n;");
                        std::string mo_encoding = extract(mo->value(0).first,"charset="," \r\n;");
                        if(mo_encoding.empty())
                            throw std::runtime_error("Invalid mo-format, encoding is not specified");
                        if(!plural.empty()) {
                            std::auto_ptr<lambda::plural> ptr=lambda::compile(plural.c_str());
                            plural_forms_[id] = ptr;
                        }
                        if( sizeof(CharType) == 1
                            && compare_encodings(mo_encoding,inf.encoding) == 0
                            && mo->has_hash())
                        {
                            mo_catalogs_[id]=mo;
                        }
                        else {
                            if(!inf.set_encoding(mo_encoding)) {
                                throw std::runtime_error("Invalid mo encoding");
                            }
                            for(unsigned i=0;i<mo->size();i++) {
                                mo_file::pair_type tmp = mo->value(i);
                                catalogs_[id][mo->key(i)]=inf.convert(tmp.first,tmp.second);
                            }
                        }
                        return true;

                    }
                    catch(std::exception const &/*err*/)
                    {
                        plural_forms_[id].reset();
                        catalogs_[id].clear();
                        return false;
                    }

                }

                int compare_encodings(std::string const &left,std::string const &right)
                {
                    return convert_encoding_name(left).compare(convert_encoding_name(right)); 
                }
                std::string convert_encoding_name(std::string const &in)
                {
                    std::string result;
                    for(unsigned i=0;i<in.size();i++) {
                        char c=in[i];
                        if('A' <= c && c<='Z')
                            c=c-'A' + 'a';
                        else if(('a' <= c && c<='z') || ('0' <= c && c<='9'))
                            ;
                        else
                            continue;
                        result+=c;
                    }
                    return result;
                }

                static std::string extract(std::string const &meta,std::string const &key,char const *separator)
                {
                    size_t pos=meta.find(key);
                    if(pos == std::string::npos)
                        return "";
                    pos+=key.size(); /// size of charset=
                    size_t end_pos = meta.find_first_of(separator,pos);
                    return meta.substr(pos,end_pos - pos);
                }

                pair_type get_string(int domain_id,char const *context,char const *in_id) const
                {
                    char const *id = in_id;
                    std::string cid;

                    if(context!=0 && *context!=0) {
                        cid.reserve(strlen(context) + strlen(in_id) + 1);
                        cid.append(context);
                        cid+='\4'; // EOT
                        cid.append(in_id);
                        id = cid.c_str();
                    }

                    pair_type null_pair((CharType const *)0,(CharType const *)0);
                    if(domain_id < 0 || size_t(domain_id) >= catalogs_.size())
                        return null_pair;
                    if(mo_catalogs_[domain_id]) {
                        mo_file::pair_type p=mo_catalogs_[domain_id]->find(id);
                        return pair_type(reinterpret_cast<CharType const *>(p.first),
                                         reinterpret_cast<CharType const *>(p.second));
                    }
                    else {
                        catalog_type const &cat = catalogs_[domain_id];
                        typename catalog_type::const_iterator p = cat.find(id);
                        if(p==cat.end())
                            return null_pair;
                        return pair_type(p->second.data(),p->second.data()+p->second.size());
                    }
                }

                catalogs_set_type catalogs_;
                std::vector<boost::shared_ptr<mo_file> > mo_catalogs_;
                std::vector<boost::shared_ptr<lambda::plural> > plural_forms_;
                domains_map_type domains_;

                
            };
        } /// impl

        
        //
        // facet IDs and specializations
        //

        std::locale::id base_message_format<char>::id;

        template<>
        message_format<char> *create_messages_facet(messages_info<char> &info)
        {
            return new impl::mo_message<char>(info);
        }

        #ifndef BOOST_NO_STD_WSTRING
        std::locale::id base_message_format<wchar_t>::id;
        
        template<>
        message_format<wchar_t> *create_messages_facet(messages_info<wchar_t> &info)
        {
            return new impl::mo_message<wchar_t>(info);
        }
        #endif
        
        #ifdef BOOST_HAS_CHAR16_T
        std::locale::id base_message_format<char16_t>::id;

        template<>
        message_format<char16_t> *create_messages_facet(messages_info<char16_t> &info)
        {
            return new impl::mo_message<char16_t>(info);
        }
        #endif
        
        #ifdef BOOST_HAS_CHAR32_T
        std::locale::id base_message_format<char32_t>::id;

        template<>
        message_format<char32_t> *create_messages_facet(messages_info<char32_t> &info)
        {
            return new impl::mo_message<char32_t>(info);
        }
        #endif

    }
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

