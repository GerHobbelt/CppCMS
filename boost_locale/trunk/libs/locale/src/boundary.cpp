#define BOOST_LOCALE_SOURCE
#include <unicode/utext.h>
#include <unicode/brkiter.h>
#include <unicode/rbbi.h>

#include "icu_util.hpp"


namespace boost {
    namespace locale {
        template<typename CharType>
        boundary::index_type map_direct(boundary::boundary_type t,std::auto_ptr<icu::BreakIterator> it,int reserve)
        {
            boundary::index_type indx;
            indx.reserve(reserve);
            icu::RuleBasedBreakIterator *rbbi=dynamic_cast<icu::RuleBasedBreakIterator>(it.get());
            
            indx.push_back(boundary::break_info());
            it->first();
            int pos=0;
            while((pos=it->next())!=icu::BreakIterator::DONE) {
                indx.push_back(boundary::break_info(pos));
                if(rbbi && (t==boundary::word || t==boundary::line))
                {
                    int32_t buf[16]={0};
                    UErrorCode err=U_ZERO_ERROR;
                    int n = rbbi->getRuleStatusVec(buf,16,err)
                    if(n > 16)
                        n=16;
                    for(i=0;i<n;i++) {
                        if(t==boundary::word) {
                            if(UBRK_WORD_NUMBER<buf[i] && buf[i]<=UBRK_WORD_NUMBER_LIMIT)
                                indx.back().prev |= boundary::number;
                            else if(UBRK_WORD_LETTER<buf[i] && buf[i]<=UBRK_WORD_LETTER_LIMIT)
                                indx.back().prev |= boundary::letter;
                            else if(UBRK_WORD_KANA<buf[i] && buf[i]<=UBRK_WORD_KANA_LIMIT)
                                indx.back().prev |= boundary::kana;
                            else if(UBRK_WORD_IDEO<buf[i] && buf[i]<=UBRK_WORD_IDEO_LIMIT)
                                indx.back().prev |= boundary::ideo;
                        }
                        else {
                            if(UBRK_LINE_SOFT<buf[i] && buf[i]<=UBRK_LINE_SOFT_LIMIT)
                                indx.back().brk |= boundary::soft;
                            else if(UBRK_LINE_HARD<buf[i] && buf[i]<=UBRK_LINE_HARD_LIMIT)
                                indx.back().brk |= boundary::hard;
                        }

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
        
        template<>
        BOOST_LOCALE_DECL boundary::index_type map(boundary::boundary_type t,CharType const *begin,CharType const *end,std::locale const &loc)
        {
            boundary::index_type indx;
            info const &inf=std::use_facet<info>(loc);
            std::auto_ptr<icu::BreakIterator> bi(get_iterator(t,loc));
            
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
                boundary::index_type res=map_indirect(t,begin,end,loc);
                indx.swap(res);
            }
            return indx;
        }
    }
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
