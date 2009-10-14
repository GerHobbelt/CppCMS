#include "mo_file.h"
namespace boost {
namespace locale {
namespace impl {

    na
    struct mo_file::header {
        uint32_t magic;
        uint32_t revision;
        uint32_t strings_no;
        uint32_t strings_offset;
        uint32_t translations_offset;
        uint32_t hash_size;
        uint32_t hash_offset;
    };

    namespace {
        struct string_data {
            uint32_t len;
            uint32_t offset;
        };
    }

    mo_file::mo_file(std::string file_name)
    {
        std::ifstream mo;
        mo.exceptions ( std::ifstream::failbit | std::ifstream::badbit );

        mo.open(file_name.c_str(),std::ifstream::binary);
        if(!mo) {
            throw std::runtime_error("Failed to open file:" + file_name);
        }
        
        header hdr;
        mo.read(reinterpret_cast<char *>&hdr,sizeof(hdr));
        if(!mo) 
            throw std::runtime_error("Failed to read header");
        if((hdr.magic != 0x950412de  && hdr.magic != 0xde120495) || hdr.revision!=0) 
            throw std::runtime_error("Invalid file format");
        if(hdr.magic == 0x950412de)
            revert_=true;
        else
            revert_=false;

        uint32_t strings_no = get(hdr.strings_no);
        uint32_t strings_offset = get(hdr.strings_offset);
        uint32_t translations_offset = get(hdr.translations_offset);
       
        mo.seekg(hdr.strings_offset);
    }

    uint32_t mo_file::get(uint32_t v)
    {
        if(!revert_) return v;
        return((v & 0xFF) << 24)
            | ((v & 0xFF00) << 8)
            | ((v & 0xFF0000) >> 8)
            | ((v & 0xFF000000) >> 24);
    }




    }

}
} 
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
