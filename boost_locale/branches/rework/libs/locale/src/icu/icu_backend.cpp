namespace boost {
namespace locale {
		
	class icu_facets_generator : public facets_generator {
	public:
		std::locale::facet *create(locale_category_type category,character_facet_type char_type) const
		{
			if(category == calendar_facet) 
			else if(category == information_facet )
			else if(category 
			else {
			}

		}
		~icu_facets_generator()
		{
		}
		template<typename CharType>
		std::locale::facet *generate(locale_category_type category)
		{
			switch(category) {
			case collation_facet:
				return new impl::collate_impl<CharType>(locale_,encoding_);
			case formatting_facet:
				return new num_format<CharType>(locale_,encoding_);
			case parsing_facet:
				return new num_parse<CharType>(locale_,encoding_);
			case boundary_facet:
				return new boundary::boundary_indexing_impl<CharType>(locale_,encoding_); 
			case message_facet: 
				...
			}
			return 0;
		}
	private:
		icu::Locale locale_;
		std::string encoding_;
		std::vector<std::string> paths_;
		std::vector<std::string> domains_;
	};

	class icu_localization_backend : public localization_backend {
	public:
		
	private:
		std::vector<std::string> paths;
		std::vector<std::string> domains;
		std::string locale_id_;
		std::string encoding_;
	};


} 
}
