#ifndef BOOST_LOCALE_CODEPAGE_HPP_INCLUDED
#define BOOST_LOCALE_CODEPAGE_HPP_INCLUDED

namespace boost {
    namespace locale {
        namespace details {
            class BOOST_LOCALE_DECL converter {
            public:
                static const uint32_t illegal=0xFFFFFFFF;
                static const uint32_t incomplete=0xFFFFFFFE;

                int max_len()
                {
                    if(is_utf8_)
                        return 4;
                    else
                        return charset_maxlen();
                }
                uint32_t to_unicode(char const *&begin,char const *end)
                {
                    if(is_utf8_)
                        return from_utf8(begin,end);
                    return from_charset(begin,end);
                }
            private:
                uint32_t from_charset(char const *&begin,char const *end)

                uint32_t to_utf8(uint32_t u,char *&begin,char const *end)
                {
                    if(u>0x10ffff)
                        return illegal;
                    if(0xd800 <=u && u<= 0xdfff) // surragates
                        return illegal;
                    ptrdiff_t d=end-begin;
                    if(u <=0x7F) {
                        if(d>=1)
                            *begin++=u;
                        else
                            return incomplete;
                    }
                    else if(u <=0x7FF) {
                        if(d>=2) {
                            *begin++=(value >> 6) | 0xC0;
                            *begin++=(value & 0x3F) | 0x80;
                        }
                        out.len=2;
                    }
                    else if(value <=0xFFFF) {
                        out.c[0]=(value >> 12) | 0xE0;
                        out.c[1]=(value >> 6) & 0x3F | 0x80;
                        out.c[2]=value & 0x3F | 0x80;
                        out.len=3;
                    }
                    else {
                        out.c[0]=(value >> 18) | 0xF0;
                        out.c[1]=(value >> 12) & 0x3F | 0x80;
                        out.c[2]=(value >> 6) & 0x3F | 0x80;
                        out.c[3]=value & 0x3F | 0x80;
                        out.len=4;
                    }
                return out;

                }
                uint32_t from_utf8(char const *&begin,char const *end)
                {
                    unsigned char const *p=reinterpret_cast<unsigned char const *>(begin);
                    unsigned char const *e=reinterpret_cast<unsigned char const *>(end);
                    if(p==e)
                        return incomplete;
                    unsigned char c=*p++;
                    unsigned char seq0,seq1=0,seq2=0,seq3=0;
                    seq0=c;
                    int len=1;
                    if((c & 0xC0) == 0xC0) {
                        if(p==e)
                            return incomplete;
                        seq1=*p++;
                        len=2;
                    }
                    if((c & 0xE0) == 0xE0) {
                        if(p==e)
                            return incomplete;
                        seq2=*p++;
                        len=3;
                    }
                    if((c & 0xF0) == 0xF0) {
                        if(p==e)
                            return incomplete;
                        seq3=*p++;
                        len=4;
                    }
                    switch(len) {
                    case 1:
                        break;
                    case 2: // non-overloading 2 bytes
                        if( 0xC2 <= seq0 && seq0 <= 0xDF
                            && 0x80 <= seq1 && seq1<= 0xBF)
                        {
                                break;
                        }
                        return illegal;
                    case 3: 
                        if(seq0==0xE0) { // exclude overloadings
                            if(0xA0 <=seq1 && seq1<= 0xBF && 0x80 <=seq2 && seq2<=0xBF)
                                break;
                        }
                        else if( (0xE1 <= seq0 && seq0 <=0xEC) || seq0==0xEE || seq0==0xEF) { // stright 3 bytes
                            if(0x80 <=seq1 && seq1<=0xBF &&
                               0x80 <=seq2 && seq2<=0xBF)
                                break;
                        }
                        else if(seq0 == 0xED) { // exclude surrogates
                            if(    0x80 <=seq1 && seq1<=0x9F &&
                                0x80 <=seq2 && seq2<=0xBF)
                                break;
                        }
                        return illegal;
                    case 4:
                        switch(seq0) {
                        case 0xF0: // planes 1-3
                            if(    0x90 <=seq1 && seq1<=0xBF &&
                                0x80 <=seq2 && seq2<=0xBF &&
                                0x80 <=seq3 && seq3<=0xBF)
                                break;
                            return illegal;
                        case 0xF1: // planes 4-15
                        case 0xF2:
                        case 0xF3:
                            if(    0x80 <=seq1 && seq1<=0xBF &&
                                0x80 <=seq2 && seq2<=0xBF &&
                                0x80 <=seq3 && seq3<=0xBF)
                                break;
                            return illegal;
                        case 0xF4: // pane 16
                            if(    0x80 <=seq1 && seq1<=0x8F &&
                                0x80 <=seq2 && seq2<=0xBF &&
                                0x80 <=seq3 && seq3<=0xBF)
                                break;
                            return utf::illegal;
                        default:
                            return utf::illegal;
                        }
                    }
                    begin=reinterpret_cast<char const *>(p);
                    switch(len) {
                    case 1:
                        return seq0;
                    case 2:
                        return ((seq0 & 0x1F) << 6) | (seq1 & 0x3F);
                    case 3:
                        return ((seq0 & 0x0F) << 12) | ((seq1 & 0x3F) << 6) | (seq2 & 0x3F)  ;
                    case 4:
                        return ((seq0 & 0x07) << 18) | ((seq1 & 0x3F) << 12) | ((seq2 & 0x3F) << 6) | (seq3 & 0x3F) ;
                    }
                }
                bool is_utf8_;
            };
        } // details
    }
}


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

