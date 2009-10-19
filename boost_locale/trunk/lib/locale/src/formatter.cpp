#include <unicode/numfmt.h>
#include <unicode/rbnf.h>
#include <unicode/datefmt.h>

namespace boost {
namespace locale {
    namespace impl {

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
                return std(tmp);
            }
            virtual string_type format(int64_t value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(value,tmp);
                code_points=tmp.countChar32();
                return std(tmp);
            }

            virtual string_type format(uint64_t value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(static_cast<int64_t>(value),tmp);
                code_points=tmp.countChar32();
                return std(tmp);
            }
            virtual string_type format(int32_t value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(value,tmp);
                code_points=tmp.countChar32();
                return std(tmp);
            }
            virtual string_type format(uint32_t value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(value,tmp);
                code_points=tmp.countChar32();
                return std(tmp);
            }

            virtual bool parse(string_type &str,double &value) const 
            {
                return false;
            }

            virtual bool parse(string_type &str,int64_t &value) const 
            {
                return false;
            }
            virtual bool parse(string_type &str,uint64_t &value) const
            {
                return false;
            }
            virtual bool parse(string_type &str,int32_t &value) const
            {
                return false;
            }
            virtual bool parse(string_type &str,uint32_t &value) const
            {
                return false;
            }

            number_format(std::auto_ptr<icu::NumberFormat> fmt,std::string codepage) :
                cvt_(codepage),
                icu_fmt_(fmt)
            {
            }
 
        private:
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

            virtual bool parse(string_type &str,double &value) const 
            {
                return false;
            }

            virtual bool parse(string_type &str,int64_t &value) const 
            {
                return false;
            }
            virtual bool parse(string_type &str,uint64_t &value) const
            {
                return false;
            }
            virtual bool parse(string_type &str,int32_t &value) const
            {
                return false;
            }
            virtual bool parse(string_type &str,uint32_t &value) const
            {
                return false;
            }

            date_format(std::auto_ptr<icu::DateFormat> fmt,std::string codepage) :
                cvt_(codepage),
                icu_fmt_(fmt)
            {
            }
 
        private:
            string_type do_format(double value,size_t &codepoints) const 
            {
                UDate date = value * 1000.0; /// UDate is time_t in miliseconds
                icu::UnicodeString tmp;
                icu_fmt_->format(value,tmp);
                code_points=tmp.countChar32();
                return std(tmp);
            }
            icu_std_converter<CharType> cvt_;
            std::auto_ptr<icu::DateFormat> icu_fmt_;
        };
        
        template<CharType>
        std::auto_ptr<formatter<CharType> > generate(std::ios_base &ios)
        {
            std::auto_ptr<formatter<CharType> > fmt;
            ios_info &info=ios_prop<ios_info>::get(ios);
            uint64_t disp = info.flags() &  flags::display_flags_mask;


            if(disp == posix)
                return fmt;
            
            icu::Locale const &locale = std::use_facet<icu_locale>(ios.getloc());
            std::string encoding;

            UErrorCode err=U_ZERO_ERROR;
            
            switch(disp) {
            case number:
                {
                    std::ios_base::fmtflags how = 
                        (ios.flags() & std::ios_base::floatfield) == std::ios_base::scientific;
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
                    uint64_t curr = info.flags() & currency_flags_mask;
                    std::auto_ptr<icu::NumberFormat> nf;
                    
                    #if U_ICU_VERSION_MAJOR_NUM*100 + U_ICU_VERSION_MINOR_NUM >= 402
                    // ICU 4.2 has special ISO currency style
                    if(curr == currency_default || curr == currency_national)
                        nf.reset(icu::NumberFormat::createInstance(locale,kIsoCurrencyStyle,err));
                    else
                        nf.reset(icu::NumberFormat::createInstance(locale,kCurrencyStyle,err))
                    if(U_FAILURE(err))
                        return fmt;

                    #else
                    //
                    // We can get currency for DecimalFormatSymbols (if number format is instance of DecimalFormat
                    // otherwise we can get this ISO formatter from standard C++ money facet
                    //
                    nf.reset(icu::NumberFormat::createCurrencyInstance(locale,err));
                    if(U_FAILURE(err))
                        return fmt;
                    err=U_ZERO_ERROR;
                    icu::DecimalFormat *df;
                    if(curr == currency_default || curr == currency_national){
                        if((df=dynamic_cast<icu::DecimalFormat>(nf.get()))!=0) {
                            icu::UnicodeString tmp =
                                df->getDecimalFormatSymbols()->getSymbol(icu::DecimalFormatSymbols::lIntlCurrencySymbol);
                            nf->setCurrecny(tmp.getBuffer(),err);
                            if(U_FAILURE(err))
                                return fmt;
                        }
                        else if(std::has_facet<std::moneypunct<CharType,true> >(ios.getloc())) {
                            std::basic_string<CharType> tmp = 
                                std::use_facet<std::moneypunct<CharType,true> >(ios.getloc()).cur_symbol();
                            if(tmp.size()!=3)
                                return fmt;
                            UChar iso[3];
                            std::copy(tmp.begin(),tmp.end(),iso);
                            nf->setCurrecny(tmp.getBuffer(),err);
                            if(U_FAILURE(err))
                                return fmt;
                        }
                        else 
                            return fmt;
                    }
                    #endif
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
                    if(!info.time_zone().empty())
                        df->adoptTimeZone(icu::TimeZone::createTimeZone(info.time_zone().c_str()));
                        
                    fmt.reset(new date_format<CharType>(df,encoding));
                }
            }

            fmt(new number_format<CharType>(nf,encodimg))
            return fmt;
        }


    } // impl


} // locale
} // boost


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4



