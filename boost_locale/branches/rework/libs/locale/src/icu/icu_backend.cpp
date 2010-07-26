namespace boost {
namespace locale {
		
	class icu_localization_backend : public localization_backend {
	public:
		icu_localization_backend() {}
		icu_localization_backend(icu_localization_backend const &other) : 
			locale_id_(other.locale_id_),
			domains_(other.domains_),
			paths_(other.paths_)
		{
		}
		virtual icu_localization_backend *clone()
		{
			return new icu_localization_backend(*this);
		}

		void set_option(std::string const &name,std::string const &value) 
		{
			if(name=="locale")
				locale_id_ = value;
			else if(name=="message_path")
				paths_.push_back(value);
			else if(name=="message_application")
				domains_.push_back(value);
		}
		void clear_options()
		{
			locale_id_.clear();
			paths_.clear();
			domains_.clear();
		}
		
            	virtual std::locale install(std::locale const &base,locale_category_type category,character_facet_type type = nochar_facet)
		{
		}
	private:
		std::vector<std::string> paths_;
		std::vector<std::string> domains_;
		std::string locale_id_;
	};


} 
}
