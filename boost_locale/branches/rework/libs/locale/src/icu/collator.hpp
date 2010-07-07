//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/locale/collator.hpp>
#include <boost/locale/info.hpp>
#include <vector>
#include <limits>

#include "info_impl.hpp"
#include "uconv.hpp"
#include "mo_hash.hpp"

#include <unicode/coll.h>

namespace boost {
    namespace locale {
        namespace impl {
            template<typename CharType>
            class collate_impl : public collator<CharType> 
            {
            public:
                typedef typename collator<CharType>::level_type level_type;
                level_type limit(level_type level) const
                {
                    if(level < 0)
                        level=collator_base::primary;
                    else if(level >= level_count)
                        level = static_cast<level_type>(level_count - 1);
                    return level;
                }
                
                virtual int do_compare( level_type level,
                                        CharType const *b1,CharType const *e1,
                                        CharType const *b2,CharType const *e2) const
                {
                    icu::UnicodeString left=cvt_.icu(b1,e1);
                    icu::UnicodeString right=cvt_.icu(b2,e2);
                    UErrorCode status=U_ZERO_ERROR;
                    std::auto_ptr<icu::Collator> collate(collates_[limit(level)]->safeClone());
                    int res = collate->compare(left,right,status);
                    if(U_FAILURE(status))
                            throw std::runtime_error(std::string("Collation failed:") + u_errorName(status));
                    if(res < 0)
                        return -1;
                    else if(res > 0)
                        return 1;
                    return 0;
                }
               
                std::vector<uint8_t> do_basic_transform(level_type level,CharType const *b,CharType const *e) const 
                {
                    icu::UnicodeString str=cvt_.icu(b,e);
                    std::vector<uint8_t> tmp;
                    tmp.resize(str.length());
                    std::auto_ptr<icu::Collator> collate(collates_[limit(level)]->safeClone());
                    int len = collate->getSortKey(str,&tmp[0],tmp.size());
                    if(len > int(tmp.size())) {
                        tmp.resize(len);
                        collate->getSortKey(str,&tmp[0],tmp.size());
                    }
                    else 
                        tmp.resize(len);
                    return tmp;
                }
                std::basic_string<CharType> do_transform(level_type level,CharType const *b,CharType const *e) const
                {
                    std::vector<uint8_t> tmp = do_basic_transform(level,b,e);
                    return std::basic_string<CharType>(tmp.begin(),tmp.end());
                }
                
                long do_hash(level_type level,CharType const *b,CharType const *e) const
                {
                    std::vector<uint8_t> tmp = do_basic_transform(level,b,e);
                    tmp.push_back(0);
                    return pj_winberger_hash_function(reinterpret_cast<char *>(&tmp.front()));
                }

                collate_impl(icu::Locale const &locale,std::string encoding) : cvt_(encoding)
                {
                
                    static const icu::Collator::ECollationStrength levels[level_count] = 
                    { 
                        icu::Collator::PRIMARY,
                        icu::Collator::SECONDARY,
                        icu::Collator::TERTIARY,
                        icu::Collator::QUATERNARY,
                        icu::Collator::IDENTICAL
                    };
                    
                    for(int i=0;i<level_count;i++) {

                        UErrorCode status=U_ZERO_ERROR;

                        collates_[i].reset(icu::Collator::createInstance(locale,status));

                        if(U_FAILURE(status))
                            throw std::runtime_error(std::string("Creation of collate failed:") + u_errorName(status));

                        collates_[i]->setStrength(levels[i]);
                    }
                }

            private:
                static const int level_count = 5;
                icu_std_converter<CharType>  cvt_;
                std::auto_ptr<icu::Collator> collates_[level_count];
            };

        } /// impl

    } // locale
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
