///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_XSS_H
#define CPPCMS_XSS_H

#include <booster/copy_ptr.h>
#include <booster/regex.h>
#include <cppcms/defs.h>

#include <string.h>
#include <string>
#include <algorithm>

namespace cppcms {
	///
	/// \brief Namespace that holds Anti-Cross Site Scripting Filter support
	///
	/// The classes in this namespace created to provide a filtering for a save
	/// handing of HTML and preventing XSS attacks
	///
	namespace xss {
		
		/// \cond INTERNAL
		namespace details {
	  
			class c_string {
			public:
				
				typedef char const *const_iterator;
				
				char const *begin() const
				{
					return begin_;
				}
				
				char const *end() const
				{
					return end_;
				}
				
				c_string(char const *s) 
				{
					begin_=s;
					end_=s+strlen(s);
				}

				c_string(char const *b,char const *e) : begin_(b), end_(e) {}

				c_string() : begin_(0),end_(0) {}

				bool compare(c_string const &other) const
				{
					return std::lexicographical_compare(begin_,end_,other.begin_,other.end_,std::char_traits<char>::lt);
				}

				bool icompare(c_string const &other) const
				{
					return std::lexicographical_compare(begin_,end_,other.begin_,other.end_,ilt);
				}

				explicit c_string(std::string const &other)
				{
					container_ = other;
					begin_ = container_.c_str();
					end_ = begin_ + container_.size();
				}
				c_string(c_string const &other)
				{
					if(other.begin_ == other.end_) {
						begin_ = end_ = 0;
					}
					else if(other.container_.empty()) {
						begin_ = other.begin_;
						end_ = other.end_;
					}
					else {
						container_ = other.container_;
						begin_ = container_.c_str();
						end_ = begin_ + container_.size();
					}
				}
				c_string const &operator=(c_string const &other)
				{
					if(other.begin_ == other.end_) {
						begin_ = end_ = 0;
					}
					else if(other.container_.empty()) {
						begin_ = other.begin_;
						end_ = other.end_;
					}
					else {
						container_ = other.container_;
						begin_ = container_.c_str();
						end_ = begin_ + container_.size();
					}
					return *this;
				}

			private:
				static bool ilt(char left,char right)
				{
					unsigned char l = tolower(left);
					unsigned char r = tolower(right);
					return l < r;
				}
				static char tolower(char c)
				{
					if('A' <= c && c<='Z')
						return c-'A' + 'a';
					return c;
				}
				char const *begin_;
				char const *end_;
				std::string container_;
			};
			
		} // details
		
		struct basic_rules_holder;
		
		/// \endcond

		///
		/// \brief The class that holds XSS filter rules
		///
		class CPPCMS_API rules {
		public:
			rules();
			rules(rules const &);
			rules const &operator=(rules const &);
			~rules();

			///
			/// How to treat in input
			///
			typedef enum {
				xhtml_input, ///< Assume that the input is XHTML
				html_input   ///< Assume that the input is HTML
			} html_type;
			
			///
			/// The type of tag
			///
			typedef enum {
				invalid_tag		= 0, ///< This tag is invalid (returned by validate)
				opening_and_closing 	= 1, ///< This tag should be opened and closed like em	, or strong
				stand_alone 		= 2, ///< This tag should stand alone (like hr or br)
				any_tag  		= 3, ///< This tag can be used in both roles (like input)
			} tag_type;

			///
			/// Get how to treat input - HTML or XHTML
			///
			html_type html() const;
			///
			/// Set how to treat input - HTML or XHTML, it should be called first before you add any other
			/// rules
			///
			void html(html_type t);

			///
			/// Add the tag that should be allowed to appear in the text, for HTML the name is case
			/// insensitive, i.e.  "br", "Br", "bR" and "BR" are valid tags for name "br".
			///
			/// The \a name should be ASCII only
			///
			void add_tag(std::string const &name,tag_type = any_tag);

			///
			/// Add allowed HTML entity, by default only "lt", "gt", "quot" and "amp" are allowed
			///
			void add_entity(std::string const &name);


			///
			/// Get if numeric entities are allowed, default is false
			///
			bool numeric_entities_allowed() const;

			///
			/// Set if numeric entities are allowed
			///
			void numeric_entities_allowed(bool v);

			///
			/// Add the property that should be allowed to appear for specific tag as boolean property like
			/// checked="checked", when the type
			/// is HTML it is case insensitive.
			///
			/// The \a property should be ASCII only
			///
			void add_boolean_property(std::string const &tag_name,std::string const &property);
			///
			/// Add the property that should be checked using regular expression.
			///
			void add_property(std::string const &tag_name,std::string const &property,booster::regex const &r);
			///
			/// Add numeric property, same as add_property(tag_name,property,booster::regex("-?[0-9]+")
			///
			void add_integer_property(std::string const &tag_name,std::string const &property);

			///
			/// Check if the comments are allowed in the text
			///
			bool comments_allowed() const;
			///
			/// Set to true if the comments are allowed in the text
			///
			void comments_allowed(bool comments);

			///
			/// Test if the tag is valid.
			/// \a tag should be lower case for HTML or unchanged for XHTML
			///
			tag_type valid_tag(details::c_string const &tag) const;
		
			///
			/// Test if the property is valid (without value) or unchanged for XHTML 
			/// \a tag and \a property should be lower case for HTML or unchanged for XHTML
			///	
			bool valid_boolean_property(details::c_string const &tag,details::c_string const &property) const;
			///
			/// Test if the property and its \a value are valid;
			///
			/// \a tag and \a property should be lower case for HTML or unchanged for XHTML
			///	
			bool valid_property(details::c_string const &tag,details::c_string const &property,details::c_string const &value) const;

			///
			/// Test if specific html entity is valid
			///
			bool valid_entity(details::c_string const &val) const;


		private:
			basic_rules_holder &impl();
			basic_rules_holder const &impl() const;

			struct data;
			booster::copy_ptr<data> d;

		};
		
		///
		/// \brief The enumerator that defines filtering invalid HTML method
		///
		typedef enum {
			remove_invalid, ///< Remove all invalid HTML form the input
			escape_invalid  ///< Escape (convert to text) all invalid HTML in the input
		} filtering_method_type;

		///
		/// \brief Check the input in range [\a begin, \a end) according to the rules \a r.
		///
		/// It does not filters the input it only checks its validity, it would be faster then validate_and_filter_if_invalid
		/// or filter functions but it does not correct errors.
		///
		CPPCMS_API bool validate(char const *begin,char const *end,rules const &r);
		///
		/// \brief Validate the input in range [\a begin, \a end) according to the rules \a r and if it is not valid filter it
		/// and save filtered text into \a filtered string using a filtering method \a method.
		///
		/// If the data was valid, \a filtered remains unchanged and the function returns true, otherwise it returns false
		/// and the filtered data is saved.
		///
		CPPCMS_API bool validate_and_filter_if_invalid(	char const *begin,
								char const *end,
								rules const &r,
								std::string &filtered,
								filtering_method_type method=remove_invalid);

		///
		/// \brief Filter the input in range [\a begin, \a end) according to the rules \a r using filtering 
		/// method \a method
		///
		CPPCMS_API std::string filter(char const *begin,
					      char const *end,
					      rules const &r,
					      filtering_method_type method=remove_invalid);
		///
		/// \brief Filter the input text \a input according to the rules \a r using filtering method \a method
		///
		CPPCMS_API std::string filter(std::string const &input,
					      rules const &r,
					      filtering_method_type method=remove_invalid);

	} // xss
}
#endif
