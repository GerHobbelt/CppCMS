#define BOOST_LOCALE_SOURCE
#include <boost/locale/formatting.hpp>
#include <boost/locale/formatter.hpp>
#include <boost/locale/info.hpp>
#include "ios_prop.hpp"
#include "formatting_info.hpp"
#include "uconv.hpp"
#include "info_impl.hpp"


#include <unicode/numfmt.h>
#include <unicode/rbnf.h>
#include <unicode/datefmt.h>
#include <unicode/decimfmt.h>

#include <limits>

namespace boost {
namespace locale {
    namespace impl {
        
        typedef ios_prop<ios_info> ios_prop_info_type;

        namespace {
            struct  initializer {
                initializer()
                {
                    /// make sure xalloc called
                    ios_prop_info_type::global_init();
                }
            } init;
        }

        template<typename CharType>
        class number_format : public formatter<CharType> {
        public:
            typedef CharType char_type;
            typedef std::basic_string<CharType> string_type;
            
            virtual string_type format(double value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(value,tmp);
                code_points=tmp.countChar32();
                return cvt_.std(tmp);
            }
            virtual string_type format(int64_t value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(value,tmp);
                code_points=tmp.countChar32();
                return cvt_.std(tmp);
            }

            virtual string_type format(uint64_t value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(static_cast<int64_t>(value),tmp);
                code_points=tmp.countChar32();
                return cvt_.std(tmp);
            }
            virtual string_type format(int32_t value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(value,tmp);
                code_points=tmp.countChar32();
                return cvt_.std(tmp);
            }
            virtual string_type format(uint32_t value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(static_cast<int64_t>(value),tmp);
                code_points=tmp.countChar32();
                return cvt_.std(tmp);
            }

            virtual size_t parse(string_type const &str,double &value) const 
            {
                return do_parse(str,value);
            }

            virtual size_t parse(string_type const &str,int64_t &value) const 
            {
                return do_parse(str,value);
            }
            virtual size_t parse(string_type const &str,uint32_t &value) const
            {
                int64_t v;
                size_t cut = do_parse(str,v);
                if(cut==0)
                    return 0;
                if(v < 0 || v > std::numeric_limits<uint32_t>::max())
                    return 0;
                value=static_cast<uint32_t>(v);
                return cut;
            }
            virtual size_t parse(string_type const &str,int32_t &value) const
            {
                return do_parse(str,value);
            }

            virtual size_t parse(string_type const &str,uint64_t &value) const
            {
                double v;
                size_t cut = do_parse(str,v);
                if(cut==0)
                    return 0;
                if(v < 0 || v > std::numeric_limits<uint64_t>::max() || static_cast<uint64_t>(v) != v)
                    return 0;
                value=static_cast<uint64_t>(v);
                return cut;
            }

            number_format(std::auto_ptr<icu::NumberFormat> fmt,std::string codepage) :
                cvt_(codepage),
                icu_fmt_(fmt)
            {
            }
 
        private:
            
            bool get_value(double &v,icu::Formattable &fmt) const
            {
                UErrorCode err=U_ZERO_ERROR;
                v=fmt.getDouble(err);
                if(U_FAILURE(err))
                    return false;
                return true;
            }

            bool get_value(int64_t &v,icu::Formattable &fmt) const
            {
                UErrorCode err=U_ZERO_ERROR;
                v=fmt.getInt64(err);
                if(U_FAILURE(err))
                    return false;
                return true;
            }

            bool get_value(int32_t &v,icu::Formattable &fmt) const
            {
                UErrorCode err=U_ZERO_ERROR;
                v=fmt.getLong(err);
                if(U_FAILURE(err))
                    return false;
                return true;
            }

            template<typename ValueType>
            size_t do_parse(string_type const &str,ValueType &v) const
            {
                icu::Formattable val;
                icu::ParsePosition pp;
                icu::UnicodeString tmp = cvt_.icu(str.data(),str.data()+str.size());

                icu_fmt_->parse(tmp,val,pp);

                ValueType tmp_v;

                if(pp.getIndex() == 0 || !get_value(tmp_v,val))
                    return 0;
                size_t cut = cvt_.cut(tmp,str.data(),str.data()+str.size(),pp.getIndex());
                if(cut==0)
                    return 0;
                v=tmp_v;
                return cut;
            }

            icu_std_converter<CharType> cvt_;
            std::auto_ptr<icu::NumberFormat> icu_fmt_;
        };
        
        
        template<typename CharType>
        class date_format : public formatter<CharType> {
        public:
            typedef CharType char_type;
            typedef std::basic_string<CharType> string_type;
            
            virtual string_type format(double value,size_t &code_points) const
            {
                return do_format(value,code_points);
            }
            virtual string_type format(int64_t value,size_t &code_points) const
            {
                return do_format(value,code_points);
            }

            virtual string_type format(uint64_t value,size_t &code_points) const
            {
                return do_format(value,code_points);
            }
            virtual string_type format(int32_t value,size_t &code_points) const
            {
                return do_format(value,code_points);
            }
            virtual string_type format(uint32_t value,size_t &code_points) const
            {
                return do_format(value,code_points);
            }

            virtual size_t parse(string_type const &str,double &value) const 
            {
                return do_parse(str,value);
            }
            virtual size_t parse(string_type const &str,int64_t &value) const 
            {
                return do_parse(str,value);
            }
            virtual size_t parse(string_type const &str,uint64_t &value) const
            {
                return do_parse(str,value);
            }
            virtual size_t parse(string_type const &str,int32_t &value) const
            {
                return do_parse(str,value);
            }
            virtual size_t parse(string_type const &str,uint32_t &value) const
            {
                return do_parse(str,value);
            }

            date_format(std::auto_ptr<icu::DateFormat> fmt,std::string codepage) :
                cvt_(codepage),
                icu_fmt_(fmt)
            {
            }
 
        private:

            template<typename ValueType>
            size_t do_parse(string_type const &str,ValueType &value) const
            {
                icu::ParsePosition pp;
                icu::UnicodeString tmp = cvt_.icu(str.data(),str.data() + str.size());

                UDate udate = icu_fmt_->parse(tmp,pp);
                if(pp.getIndex() == 0)
                    return 0;
                double date = udate / 1000.0;
                typedef std::numeric_limits<ValueType> limits_type;
                if(date > limits_type::max() || date < limits_type::min())
                    return 0;
                size_t cut = cvt_.cut(tmp,str.data(),str.data()+str.size(),pp.getIndex());
                if(cut==0)
                    return 0;
                value=static_cast<ValueType>(date);
                return cut;

            }
            
            string_type do_format(double value,size_t &codepoints) const 
            {
                UDate date = value * 1000.0; /// UDate is time_t in miliseconds
                icu::UnicodeString tmp;
                icu_fmt_->format(date,tmp);
                codepoints=tmp.countChar32();
                return cvt_.std(tmp);
            }

            icu_std_converter<CharType> cvt_;
            std::auto_ptr<icu::DateFormat> icu_fmt_;
        };
        
        template<typename CharType>
        std::auto_ptr<formatter<CharType> > generate_formatter(std::ios_base &ios)
        {
            using namespace boost::locale::flags;

            std::auto_ptr<formatter<CharType> > fmt;
            ios_info &info=ios_prop<ios_info>::get(ios);
            uint64_t disp = info.flags() &  flags::display_flags_mask;


            if(disp == posix)
                return fmt;
           
            boost::locale::info const &locale_info = std::use_facet<boost::locale::info>(ios.getloc());
            icu::Locale const &locale = locale_info.impl()->locale;
            std::string encoding = locale_info.impl()->encoding;


            UErrorCode err=U_ZERO_ERROR;
            
            switch(disp) {
            case number:
                {
                    std::ios_base::fmtflags how = (ios.flags() & std::ios_base::floatfield);
                    std::auto_ptr<icu::NumberFormat> nf;

                    if(how == std::ios_base::scientific)
                        nf.reset(icu::NumberFormat::createScientificInstance(locale,err));
                    else
                        nf.reset(icu::NumberFormat::createInstance(locale,err));
                    if(U_FAILURE(err)) {
                        return fmt;
                    }
                    if(how == std::ios_base::scientific || how == std::ios_base::fixed ) {
                        nf->setMaximumFractionDigits(ios.precision());
                        nf->setMinimumFractionDigits(ios.precision());
                    }
                    fmt.reset(new number_format<CharType>(nf,encoding));
                }
                break;
            case currency:
                {
                    std::auto_ptr<icu::NumberFormat> nf;
                    
                    #if U_ICU_VERSION_MAJOR_NUM*100 + U_ICU_VERSION_MINOR_NUM >= 402
                    //
                    // ICU 4.2 has special ISO currency style
                    //
                    
                    uint64_t curr = info.flags() & currency_flags_mask;

                    if(curr == currency_default || curr == currency_national)
                        nf.reset(icu::NumberFormat::createInstance(locale,icu::NumberFormat::kIsoCurrencyStyle,err));
                    else
                        nf.reset(icu::NumberFormat::createInstance(locale,icu::NumberFormat::kCurrencyStyle,err));

                    #else
                    //
                    // Before 4.2 we have no way to create ISO Currency 
                    //
                    
                    nf.reset(icu::NumberFormat::createCurrencyInstance(locale,err));

                    #endif

                    if(U_FAILURE(err))
                        return fmt;

                    fmt.reset(new number_format<CharType>(nf,encoding));
                }
                break;
            case percent:
                {
                    std::auto_ptr<icu::NumberFormat> nf;
                    nf.reset(icu::NumberFormat::createPercentInstance(locale,err));
                    if(U_FAILURE(err))
                        return fmt;
                    fmt.reset(new number_format<CharType>(nf,encoding));
                }
                break;
            case spellout:
            case ordinal:
                {
                    std::auto_ptr<icu::NumberFormat> nf;
                    URBNFRuleSetTag tag=URBNF_SPELLOUT;
                    if(disp==spellout)
                        tag=URBNF_SPELLOUT;
                    else // ordinal
                        tag=URBNF_ORDINAL;
                    nf.reset(new icu::RuleBasedNumberFormat(tag,locale,err));
                    if(U_FAILURE(err))
                        return fmt;
                    fmt.reset(new number_format<CharType>(nf,encoding));
                }
                break;
            case date:
            case time:
            case datetime:
                {
                    using namespace flags;
                    icu::DateFormat::EStyle dstyle = icu::DateFormat::kDefault,tstyle = icu::DateFormat::kDefault;
                    std::auto_ptr<icu::DateFormat> df;
                    switch(info.flags() & time_flags_mask) {
                    case time_short:    tstyle=icu::DateFormat::kShort; break;
                    case time_medium:   tstyle=icu::DateFormat::kMedium; break;
                    case time_long:     tstyle=icu::DateFormat::kLong; break;
                    case time_full:     tstyle=icu::DateFormat::kFull; break;
                    }
                    switch(info.flags() & date_flags_mask) {
                    case date_short:    dstyle=icu::DateFormat::kShort; break;
                    case date_medium:   dstyle=icu::DateFormat::kMedium; break;
                    case date_long:     dstyle=icu::DateFormat::kLong; break;
                    case date_full:     dstyle=icu::DateFormat::kFull; break;
                    }
                    if(disp==date)
                        df.reset(icu::DateFormat::createDateInstance(dstyle,locale));
                    else if(disp==time)
                        df.reset(icu::DateFormat::createTimeInstance(tstyle,locale));
                    else // datetime
                        df.reset(icu::DateFormat::createDateTimeInstance(dstyle,tstyle,locale));
                    if(!df.get())
                        return fmt;
                    if(!info.timezone().empty())
                        df->adoptTimeZone(icu::TimeZone::createTimeZone(info.timezone().c_str()));
                        
                    fmt.reset(new date_format<CharType>(df,encoding));
                }
                break;
            }

            return fmt;
        }

        template<typename Char>
        formatter<Char> const *get_cached_formatter(std::ios_base &ios)
        {
            return ios_prop<ios_info>::get(ios).formatter<Char>(ios);
        }


    } // impl

    template<>
    std::auto_ptr<formatter<char> > formatter<char>::create(std::ios_base &ios)
    {
        return impl::generate_formatter<char>(ios);
    }

    template<>
    formatter<char> const *formatter<char>::get(std::ios_base &ios)
    {
        return impl::get_cached_formatter<char>(ios);
    }
    #ifndef BOOST_NO_STD_WSTRING
    template<>
    std::auto_ptr<formatter<wchar_t> > formatter<wchar_t>::create(std::ios_base &ios)
    {
        return impl::generate_formatter<wchar_t>(ios);
    }

    template<>
    formatter<wchar_t> const *formatter<wchar_t>::get(std::ios_base &ios)
    {
        return impl::get_cached_formatter<wchar_t>(ios);
    }
    #endif

    #ifdef BOOST_HAS_CHAR16_T
    template<>
    std::auto_ptr<formatter<char16_t> > formatter<char16_t>::create(std::ios_base &ios)
    {
        return impl::generate_formatter<char16_t>(ios);
    }

    template<>
    formatter<char16_t> const *formatter<char16_t>::get(std::ios_base &ios)
    {
        return impl::get_cached_formatter<char16_t>(ios);
    }
    #endif

    #ifdef BOOST_HAS_CHAR32_T
    template<>
    std::auto_ptr<formatter<char32_t> > formatter<char32_t>::create(std::ios_base &ios)
    {
        return impl::generate_formatter<char32_t>(ios);
    }

    template<>
    formatter<char32_t> const *formatter<char32_t>::get(std::ios_base &ios)
    {
        return impl::get_cached_formatter<char32_t>(ios);
    }
    #endif

} // locale
} // boost


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4



