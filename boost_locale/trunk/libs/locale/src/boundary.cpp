#define BOOST_LOCALE_SOURCE
#include <boost/locale/boundary.hpp>
#include <boost/locale/info.hpp>
#include <unicode/utext.h>
#include <unicode/brkiter.h>
#include <unicode/rbbi.h>

#include "icu_util.hpp"
#include "info_impl.hpp"
#include "uconv.hpp"

namespace boost {
namespace locale {
namespace impl {

boundary::index_type map_direct(boundary::boundary_type t,icu::BreakIterator *it,int reserve)
{
    boundary::index_type indx;
    indx.reserve(reserve);
    icu::RuleBasedBreakIterator *rbbi=dynamic_cast<icu::RuleBasedBreakIterator *>(it);
    
    indx.push_back(boundary::break_info());
    it->first();
    int pos=0;
    while((pos=it->next())!=icu::BreakIterator::DONE) {
        indx.push_back(boundary::break_info(pos));
        if(rbbi && (t==boundary::word || t==boundary::line))
        {
			//
			// There is a collapse for MSVC: int32_t defined by both boost::cstdint and icu...
			// So need to pick one ;(
			//
            ::int32_t buf[16]={0};
            UErrorCode err=U_ZERO_ERROR;
            int n = rbbi->getRuleStatusVec(buf,16,err);
            if(n > 16)
                n=16;
            for(int i=0;i<n;i++) {
                if(t==boundary::word) {
                    if(UBRK_WORD_NUMBER<=buf[i] && buf[i]<UBRK_WORD_NUMBER_LIMIT)
                        indx.back().prev |= boundary::number;
                    else if(UBRK_WORD_LETTER<=buf[i] && buf[i]<UBRK_WORD_LETTER_LIMIT)
                        indx.back().prev |= boundary::letter;
                    else if(UBRK_WORD_KANA<=buf[i] && buf[i]<UBRK_WORD_KANA_LIMIT)
                        indx.back().prev |= boundary::kana;
                    else if(UBRK_WORD_IDEO<=buf[i] && buf[i]<UBRK_WORD_IDEO_LIMIT)
                        indx.back().prev |= boundary::ideo;
                }
                else {
                    if(UBRK_LINE_SOFT<=buf[i] && buf[i]<UBRK_LINE_SOFT_LIMIT)
                        indx.back().brk |= boundary::soft;
                    else if(UBRK_LINE_HARD<=buf[i] && buf[i]<UBRK_LINE_HARD_LIMIT)
                        indx.back().brk |= boundary::hard;
                }
            }
        }
    }
    if(rbbi && t==boundary::word) {
        for(unsigned i=0;i<indx.size()-1;i++)
            indx[i].next=indx[i+1].prev;
    }
    return indx;
}

std::auto_ptr<icu::BreakIterator> get_iterator(boundary::boundary_type t,std::locale const &l)
{
    icu::Locale const &loc=std::use_facet<info>(l).impl()->locale;
    UErrorCode err=U_ZERO_ERROR;
    std::auto_ptr<icu::BreakIterator> bi;
    switch(t) {
    case boundary::character:
        bi.reset(icu::BreakIterator::createCharacterInstance(loc,err));
        break;
    case boundary::word:
        bi.reset(icu::BreakIterator::createWordInstance(loc,err));
        break;
    case boundary::sentence:
        bi.reset(icu::BreakIterator::createSentenceInstance(loc,err));
        break;
    case boundary::line:
        bi.reset(icu::BreakIterator::createLineInstance(loc,err));
        break;
    default:
        throw std::runtime_error("Invalid iteration type");
    }
    check_and_throw_icu_error(err);
    if(!bi.get())
        throw std::runtime_error("Failed to create break iterator");
    return bi;
}


template<typename CharType>
boundary::index_type do_map(boundary::boundary_type t,CharType const *begin,CharType const *end,std::locale const &loc)
{
    boundary::index_type indx;
    info const &inf=std::use_facet<info>(loc);
    std::auto_ptr<icu::BreakIterator> bi(get_iterator(t,loc));
   
    UErrorCode err=U_ZERO_ERROR;
    if(sizeof(CharType) == 2 || (sizeof(CharType)==1 && inf.utf8()))
    {
        UText *ut=0;
        try {
            if(sizeof(CharType)==1)
                ut=utext_openUTF8(0,reinterpret_cast<char const *>(begin),end-begin,&err);
            else // sizeof(CharType)==2
                ut=utext_openUChars(0,reinterpret_cast<UChar const *>(begin),end-begin,&err);

            check_and_throw_icu_error(err);
            err=U_ZERO_ERROR;
            if(!ut) throw std::runtime_error("Failed to create UText");
            bi->setText(ut,err);
            check_and_throw_icu_error(err);
            boundary::index_type res=map_direct(t,bi.get(),end-begin);
            indx.swap(res);
        }
        catch(...) {
            if(ut)
                utext_close(ut);
            throw;
        }
        if(ut) utext_close(ut);
    }
    else {
        impl::icu_std_converter<CharType> cvt(inf.encoding());
        icu::UnicodeString str=cvt.icu(begin,end);
        bi->setText(str);
        boundary::index_type indirect = map_direct(t,bi.get(),str.length());
        indx=indirect;
        for(unsigned i=1;i<indirect.size();i++) {
            unsigned offset_inderect=indirect[i-1].offset;
            unsigned diff = indirect[i].offset - offset_inderect;
            unsigned offset_direct=indx[i-1].offset;
            indx[i].offset=offset_direct + cvt.cut(str,begin,end,diff,offset_inderect,offset_direct);
        }
    }
    return indx;
} // do_map

} // impl


template<>
BOOST_LOCALE_DECL boundary::index_type 
boundary::map(boundary::boundary_type t,char const *begin,char const *end,std::locale const &loc)
{
    return impl::do_map(t,begin,end,loc);
}

#ifndef BOOST_NO_STD_WSTRING
template<>
BOOST_LOCALE_DECL boundary::index_type 
boundary::map(boundary::boundary_type t,wchar_t const *begin,wchar_t const *end,std::locale const &loc)
{
    return impl::do_map(t,begin,end,loc);
}
#endif

#ifdef BOOST_HAS_CHAR16_T
template<>
BOOST_LOCALE_DECL boundary::index_type 
boundary::map(boundary::boundary_type t,char16_t const *begin,char16_t const *end,std::locale const &loc)
{
    return impl::do_map(t,begin,end,loc);
}
#endif

#ifdef BOOST_HAS_CHAR32_T
template<>
BOOST_LOCALE_DECL boundary::index_type 
boundary::map(boundary::boundary_type t,char32_t const *begin,char32_t const *end,std::locale const &loc)
{
    return impl::do_map(t,begin,end,loc);
}
#endif

} // locale
} // boost
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
