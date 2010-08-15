//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_LOCALE_SOURCE
#include <locale>
#include "all_generator.hpp"
#include <vector>
namespace boost {
namespace locale {
namespace impl_std {
    namespace details {
        class converter {
        public:

            static const uint32_t illegal=0xFFFFFFFF;
            static const uint32_t incomplete=0xFFFFFFFE;
            
            uint32_t to_unicode(char const *&begin,char const *end);
            uint32_t from_unicode(uint32_t u,char *begin,char const *end);
            converter()
            {
            }
        private:

            //
            // Non Copyable
            //
            converter(converter const &);
            void operator=(converter const &);

            uint32_t to_utf8(uint32_t u,char *begin,char const *end);
            uint32_t from_utf8(char const *&begin,char const *end);

            // Private data members
        };

        template<typename CharType,int n=sizeof(CharType)>
        struct uchar_traits;

        template<typename CharType>
        struct uchar_traits<CharType,2> {
            typedef uint16_t uint_type;
        };
        template<typename CharType>
        struct uchar_traits<CharType,4> {
            typedef uint32_t uint_type;
        };

        uint32_t converter::to_unicode(char const *&begin,char const *end)
        {
            return from_utf8(begin,end);
        }
        uint32_t converter::from_unicode(uint32_t u,char *begin,char const *end)
        {
            return to_utf8(u,begin,end);
        }


        uint32_t converter::to_utf8(uint32_t u,char *begin,char const *end)
        {
            if(u>0x10ffff)
                return illegal;
            if(0xd800 <=u && u<= 0xdfff) // surrogates
                return illegal;
            ptrdiff_t d=end-begin;
            if(u <=0x7F) { 
                if(d>=1) {
                    *begin++=u;
                    return 1;
                }
                else
                    return incomplete;
            }
            else if(u <= 0x7FF) {
                if(d>=2) {
                    *begin++=(u >> 6) | 0xC0;
                    *begin++=(u & 0x3F) | 0x80;
                    return 2;
                }
                else
                    return incomplete;
            }
            else if(u <= 0xFFFF) {
                if(d>=3) {
                    *begin++=(u >> 12) | 0xE0;
                    *begin++=((u >> 6) & 0x3F) | 0x80;
                    *begin++=(u & 0x3F) | 0x80;
                    return 3;
                }
                else
                    return incomplete;
            }
            else {
                if(d>=4) {
                    *begin++=(u >> 18) | 0xF0;
                    *begin++=((u >> 12) & 0x3F) | 0x80;
                    *begin++=((u >> 6) & 0x3F) | 0x80;
                    *begin++=(u & 0x3F) | 0x80;
                    return 4;
                }
                else
                    return incomplete;
            }
        }
        uint32_t converter::from_utf8(char const *&begin,char const *end)
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
                if(seq0==0xE0) { // exclude overloading
                    if(0xA0 <=seq1 && seq1<= 0xBF && 0x80 <=seq2 && seq2<=0xBF)
                        break;
                }
                else if( (0xE1 <= seq0 && seq0 <=0xEC) || seq0==0xEE || seq0==0xEF) { // stright 3 bytes
                    if(0x80 <=seq1 && seq1<=0xBF &&
                       0x80 <=seq2 && seq2<=0xBF)
                        break;
                }
                else if(seq0 == 0xED) { // exclude surrogates
                    if( 0x80 <=seq1 && seq1<=0x9F &&
                        0x80 <=seq2 && seq2<=0xBF)
                        break;
                }
                return illegal;
            case 4:
                switch(seq0) {
                case 0xF0: // planes 1-3
                    if( 0x90 <=seq1 && seq1<=0xBF &&
                        0x80 <=seq2 && seq2<=0xBF &&
                        0x80 <=seq3 && seq3<=0xBF)
                        break;
                    return illegal;
                case 0xF1: // planes 4-15
                case 0xF2:
                case 0xF3:
                    if( 0x80 <=seq1 && seq1<=0xBF &&
                        0x80 <=seq2 && seq2<=0xBF &&
                        0x80 <=seq3 && seq3<=0xBF)
                        break;
                    return illegal;
                case 0xF4: // pane 16
                    if( 0x80 <=seq1 && seq1<=0x8F &&
                        0x80 <=seq2 && seq2<=0xBF &&
                        0x80 <=seq3 && seq3<=0xBF)
                        break;
                    return illegal;
                default:
                    return illegal;
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
            default: // can be only 4
                return ((seq0 & 0x07) << 18) | ((seq1 & 0x3F) << 12) | ((seq2 & 0x3F) << 6) | (seq3 & 0x3F) ;
            }
        }

    } // details 
    ///
    /// \brief this class reimplements standard C++ codecvt facet. It is rarely used directly however it can be
    /// useful for code page conversions
    ///
    template<typename CharType>
    class utf8_converter : public std::codecvt<CharType,char,mbstate_t> 
    {
    public:
        utf8_converter(size_t refs = 0 ) : std::codecvt<CharType,char,mbstate_t>(refs)
        {
        }

    protected:
        typedef CharType uchar;
        
        virtual std::codecvt_base::result do_unshift(std::mbstate_t &s,char *from,char *to,char *&next) const
        {
            next=from;
            return std::codecvt_base::ok;
        }
        virtual int do_encoding() const throw()
        {
            // Always assume variable length
            return 0;
        }
        virtual int do_max_length() const throw()
        {
            return 4;
        }
        virtual bool do_always_noconv() const throw()
        {
            return false;
        }
        
        virtual std::codecvt_base::result 
        do_in(  std::mbstate_t &state,
                char const *from,
                char const *from_end,
                char const *&from_next,
                uchar *uto,
                uchar *uto_end,
                uchar *&uto_next) const
        {
            typedef typename details::uchar_traits<uchar>::uint_type uint_type;
            uint_type *to=reinterpret_cast<uint_type *>(uto);
            uint_type *to_end=reinterpret_cast<uint_type *>(uto_end);
            uint_type *&to_next=reinterpret_cast<uint_type *&>(uto_next);
            return do_real_in(state,from,from_end,from_next,to,to_end,to_next);
        }
        
        virtual int
        do_length(  std::mbstate_t &state,
                char const *from,
                char const *from_end,
                size_t max) const
        {
            char const *from_next=from;
            std::vector<uchar> chrs(max+1);
            uchar *to=&chrs.front();
            uchar *to_end=to+max;
            uchar *to_next=to;
            do_in(state,from,from_end,from_next,to,to_end,to_next);
            return from_next-from;
        }

        virtual std::codecvt_base::result 
        do_out( std::mbstate_t &state,
                uchar const *ufrom,
                uchar const *ufrom_end,
                uchar const *&ufrom_next,
                char *to,
                char *to_end,
                char *&to_next) const
        {
            typedef typename details::uchar_traits<uchar>::uint_type uint_type;
            uint_type const *from=reinterpret_cast<uint_type const *>(ufrom);
            uint_type const *from_end=reinterpret_cast<uint_type const *>(ufrom_end);
            uint_type const *&from_next=reinterpret_cast<uint_type const *&>(ufrom_next);
            return do_real_out(state,from,from_end,from_next,to,to_end,to_next);
        }
       

    private:

        //
        // Implementation for UTF-32
        //
        std::codecvt_base::result
        do_real_in( std::mbstate_t &state,
                    char const *from,
                    char const *from_end,
                    char const *&from_next,
                    uint32_t *to,
                    uint32_t *to_end,
                    uint32_t *&to_next) const
        {
            details::converter cvt;
            std::codecvt_base::result r=std::codecvt_base::ok;
            while(to < to_end && from < from_end)
            {
                uint32_t ch=cvt.to_unicode(from,from_end);
                if(ch==details::converter::illegal) {
                    r=std::codecvt_base::error;
                    break;
                }
                if(ch==details::converter::incomplete) {
                    r=std::codecvt_base::partial;
                    break;
                }
                *to++=ch;
            }
            from_next=from;
            to_next=to;
            if(r!=std::codecvt_base::ok)
                return r;
            if(from!=from_end)
                return std::codecvt_base::partial;
            return r;
        }

        //
        // Implementation for UTF-32
        //
        std::codecvt_base::result
        do_real_out(std::mbstate_t &state,
                    uint32_t const *from,
                    uint32_t const *from_end,
                    uint32_t const *&from_next,
                    char *to,
                    char *to_end,
                    char *&to_next) const
        {
            details::converter cvt;
            std::codecvt_base::result r=std::codecvt_base::ok;
            while(to < to_end && from < from_end)
            {
                uint32_t len=cvt.from_unicode(*from,to,to_end);
                if(len==details::converter::illegal) {
                    r=std::codecvt_base::error;
                    break;
                }
                if(len==details::converter::incomplete) {
                    r=std::codecvt_base::partial;
                    break;
                }
                from++;
                to+=len;
            }
            from_next=from;
            to_next=to;
            if(r!=std::codecvt_base::ok)
                return r;
            if(from!=from_end)
                return std::codecvt_base::partial;
            return r;
        }

        //
        // Can't handle full UTF-16, only UCS-2
        // because:
        //   1. codecvt facet must be able to work on single
        //      internal character ie if  do_in(s,from,from_end,x,y,z,t) returns ok
        //      then do_in(s,from,from+1) should return ok according to the standard papars
        //   2. I have absolutly NO information about mbstat_t -- I can't even know if its 0 
        //      or it is somehow initialized. So I can't store any state information
        //      about suragate pairs... So it works only for UCS-2
        //
        
        
        //
        // Implementation for UTF-16
        //
        std::codecvt_base::result
        do_real_in( std::mbstate_t &state,
                    char const *from,
                    char const *from_end,
                    char const *&from_next,
                    uint16_t *to,
                    uint16_t *to_end,
                    uint16_t *&to_next) const
        {
            details::converter cvt;
            std::codecvt_base::result r=std::codecvt_base::ok;
            while(to < to_end && from < from_end)
            {
                uint32_t ch=cvt.to_unicode(from,from_end);
                if(ch==details::converter::illegal) {
                    r=std::codecvt_base::error;
                    break;
                }
                if(ch==details::converter::incomplete) {
                    r=std::codecvt_base::partial;
                    break;
                }
                if(ch <= 0xFFFF) {
                    *to++=ch;
                }
                else { /// can't handle surrogates
                    r=std::codecvt_base::error;
                    break;
                }
            }
            from_next=from;
            to_next=to;
            if(r!=std::codecvt_base::ok)
                return r;
            if(from!=from_end)
                return std::codecvt_base::partial;
            return r;
        }

        //encoding// Implementation for UTF-16
        //
        std::codecvt_base::result
        do_real_out(std::mbstate_t &state,
                    uint16_t const *from,
                    uint16_t const *from_end,
                    uint16_t const *&from_next,
                    char *to,
                    char *to_end,
                    char *&to_next) const
        {
            details::converter cvt;
            std::codecvt_base::result r=std::codecvt_base::ok;
            while(to < to_end && from < from_end)
            {
                uint32_t ch=*from;
                if(0xD800 <= ch && ch<=0xDFFF) {
                    r=std::codecvt_base::error;
                    // Can't handle surragates
                    break;
                }
                        
                uint32_t len=cvt.from_unicode(ch,to,to_end);
                if(len==details::converter::illegal) {
                    r=std::codecvt_base::error;
                    break;
                }
                if(len==details::converter::incomplete) {
                    r=std::codecvt_base::partial;
                    break;
                }
                to+=len;
                from++;
            }
            from_next=from;
            to_next=to;
            if(r!=std::codecvt_base::ok)
                return r;
            if(from!=from_end)
                return std::codecvt_base::partial;
            return r;
        }

    };

    template<typename CharType>
    std::locale codecvt_bychar( std::locale const &in,
                                std::string const &locale_name,
                                utf8_support utf)
    {
        if(utf == utf8_from_wide)
            return std::locale(in,new utf8_converter<CharType>());
        else
            return std::locale(in,new std::codecvt_byname<CharType,char,mbstate_t>(locale_name.c_str()));
    }
    

    std::locale create_codecvt( std::locale const &in,
                                std::string const &locale_name,
                                character_facet_type type,
                                utf8_support utf) 
    {
        switch(type) {
        case char_facet:
            return std::locale(in,new std::codecvt_byname<char,char,mbstate_t>(locale_name.c_str()));
        #ifndef BOOST_NO_STD_WSTRING
        case wchar_t_facet:
            return codecvt_bychar<wchar_t>(in,locale_name,utf);
        #endif
        #if defined(BOOST_HAS_CHAR16_T) && !defined(BOOST_NO_CHAR16_T_CODECVT)
        case char16_t_facet:
            return codecvt_bychar<char16_t>(in,locale_name,utf);
        #endif
        #if defined(BOOST_HAS_CHAR32_T) && !defined(BOOST_NO_CHAR32_T_CODECVT)
        case char32_t_facet:
            return codecvt_bychar<char32_t>(in,locale_name,utf);
        #endif
        default:
            return in;
        }
    }

} // impl_std
} // locale 
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
