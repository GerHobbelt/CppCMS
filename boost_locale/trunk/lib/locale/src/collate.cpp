#include <boost/locale/collate.hpp>
#include <boost/locale/info.hpp>

#include "info_impl.hpp"

namespace boost {
    namespace locale {
        namespace impl {
            template<typename CharType>
            class collate_impl : public collate<CharType> 
            {
            public:
                level_type limit(level_type l) const
                {
                    if(level < 0)
                        level=0;
                    else if(level >= level_count)
                        level = static_cast<level_type>(level_count - 1);
                    return level;
                }
                
                int do_collate(level_type level,CharType const *b1,CharType const *e1,CarType const *b2,CharType const *e2) const
                {
                    icu::UnicodeString left=cvt_.icu(b1,e1);
                    icu::UnicodeString right=cvt_.icu(b2,e2);
                    UErrorCode status=U_ZERO_ERROR;
                    int res = collators_[limit(level)]->compare(left,right,status);
                    if(res < 0)
                        return -1;
                    else if(res > 0)
                        return 1;
                    return 0;
                }
                
                std::string do_transform(level_type level,CharType const *b,CharType const *e) const
                {
                    icu::UnicodeString str=cvt_.icu(b,e);
                    std::vector<char> tmp;
                    tmp.resize(str.length());
                    boost::shared_ptr<icu::Collator> collator=collatros_[limit(level)];
                    int len = collator->getSortKey(us,reinterpret_cast<uint8_t *>(&tmp[0]),tmp.size());
                    if(len > int(tmp.size())) {
                        tmp.resize(len);
                        collator->getSortKey(us,reinterpret_cast<uint8_t *>(&tmp[0]),tmp.size());
                    }
                    else 
                        tmp.resize(len);
                    return std::string(&tmp.front(),tmp.size()); 
                }
                
                long do_hash(level_type level,CharType const *b,CharType const *e) const
                {
                    boost::hash<std::string> hasher;
                    return hasher(do_transform(level,b,e));
                }

                collate_impl(icu::Locale const &locale,std::string encoding) : cvt_(encoding)
                {
                
                    static const icu::Collator::ECollationStrength levels[level_count] = 
                    { 
                        icu::Collator::PRIMARY,
                        icu::Collator::SECONDARY,
                        icu::Collator::TERTIARY,
                        icu::Collator::QUATERNARY
                    };
                    
                    for(int i=0;i<level_count,i++) {

                        UErrorCode status=U_ZERO_ERROR;

                        collators_[i].reset(icu::Collator::createInstance(locale,status));

                        if(U_FAILURE(status))
                            throw std::runtime_error(std::string("Creation of collator failed:") + u_errorName(status));

                        collators_[i]->setStrength(levels[i]);
                    }
                }

            private:
                icu_std_converter<CharType>  cvt_;
                boost::shared_ptr<icu::Collator> collators_[level_count];
            };

        } /// impl

        template<>
        collator<char> *collator<char>::create(info const &inf)
        {
            return new impl::collate_impl<char>(inf.impl()->locale,info.impl()->encoding);
        }
        #ifndef BOOST_NO_STD_WSTRING
        template<>
        collator<wchar_t> *collator<wchar_t>::create(info const &inf)
        {
            return new impl::collate_impl<wchar_t>(inf.impl()->locale,info.impl()->encoding);
        }
        #endif
        
        #ifdef BOOST_HAS_CHAR16_T
        template<>
        collator<char16_t> *collator<char16_t>::create(info const &inf)
        {
            return new impl::collate_impl<char16_t>(inf.impl()->locale,info.impl()->encoding);
        }
        #endif
        
        #ifdef BOOST_HAS_CHAR32_T
        template<>
        collator<char32_t> *collator<char32_t>::create(info const &inf)
        {
            return new impl::collate_impl<char32_t>(inf.impl()->locale,info.impl()->encoding);
        }
        #endif




    } // locale
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
