namespace boost {
    namespace locale {
        namespace period {
            ///
            ///
            /// This enum provides the list of various time periods that can be used for manipulation over date and time
            /// Operators like +, - * defined for these period allowing to perform easy calculations over them
            ///
            typedef enum {
                invalid,                    ///< Special invalid value, should not be used directs
                era,                        ///< Era i.e. AC, BC in Gregorian and Julian calendar, range [0,1]
                year,                       ///< Year, it is calendar specific
                extended_year,              ///< Extended year for Gregorian/Julian calendars, where 1 BC == 0, 2 BC == -1.
                month,                      ///< The month of year, calendar specific, in Gregorian [0..11]
                day,                        ///< The day of month, calendar specific, in Gregorian [1..31]
                day_of_year,                ///< The number of day in year, starting from 1
                day_of_week,                ///< Day of week, starting from Sunday, [1..7]
                day_of_week_in_month,       ///< Original number of the day of the week in month.
                day_of_week_local,          ///< Local day of week, for example in France Monday is 1, in US Sunday is 1, [1..7]
                hour,                       ///< 24 clock hour [0..23]
                hour_12,                    ///< 12 clock hour [0..11]
                am_pm,                      ///< am or pm marker, [0..1]
                minute,                     ///< minute [0..59]
                second,                     ///< second [0..59]
                week_of_year,               ///< The week number in the year
                week_of_month,              ///< The week number withing current month
            } period_type;
        } // period

        class calendar_impl {
        public:
            typedef enum {
                absolute_minimum,
                actual_minimum,
                greatest_minimum,
                current,
                least_maximum,
                actual_maximum,
                absolute_maximum,
            } value_type;

            typedef enum {
                move,
                roll,
            } update_type;

            typedef enum {
                is_gregorian,
            } calendar_option_type;

            struct posix_time {
                int64_t seconds;
                uint32_t nanoseconds;
            };


            virtual calendar_impl *clone() const = 0;

            virtual void set_current_value(period::period_type p,int value) = 0;
            virtual int get_value(period::period_type p,value_type v) const = 0;

            virtual void set_time(posix const &p)  = 0;
            virtual posix get_time() const  = 0;

            virtual void set_option(calendar_option_type opt,int v) const = 0;
            virtual int get_option(calendar_option_type opt) const = 0;

            virtual void adjust_value(period::period_type p,update_type u,int difference) = 0;

            virtual int compare(calendar_impl const *other) const = 0;
            virtual int difference(calendar_impl const *other,period::period_type p) const = 0;

            virtual void set_timezone(std::string const &tz) = 0;
            virtual std::string get_timezone() const = 0;

        };


        class BOOST_LOCALE_DECL calendar_facet : public std::locale::facet {
        public:
            calendar_facet(size_t refs = 0) : std::locale::facet(refs) 
            {
            }
            virtual calendar_impl *create_calendar(how_type how) const = 0;
            static std::locale::id id;
        };

        class BOOST_LOCALE_DECL time_zone_facet : public std::locale::facet {
        public:
            time_zone_facet(size_t refs = 0) : std::locale::facet(refs)
            {
            }
            virtual int offset_from_gmt(std::string const &id,int64_t posix_time_point) const = 0;
            virtual bool in_daylight_savings(std::string const &id) const = 0;
            virtual bool has_daylight_savings(std::string const &id,int64_t posix_time_point) const = 0;

            static std::locale::id id;
        };

    } // locale
} // boost
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

