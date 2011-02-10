#include <apps/basic_master.h>
#include <data/basic_master.h>
#include <cppdb/frontend.h>
#include <cppcms/json.h>
#include <cppcms/xss.h>
extern "C" {
	#include <mkdio.h>
}


namespace {
	cppcms::xss::rules const &xss_filter()
	{
		static cppcms::xss::rules r;
		static bool initialized = false;
		if(initialized)
			return r;
		using namespace cppcms::xss;

		r.html(rules::xhtml_input);
		r.add_tag("ol",rules::opening_and_closing);
		r.add_tag("ul",rules::opening_and_closing);
		r.add_tag("li",rules::opening_and_closing);
		r.add_tag("p",rules::opening_and_closing);
		r.add_tag("b",rules::opening_and_closing);
		r.add_tag("i",rules::opening_and_closing);
		r.add_tag("tt",rules::opening_and_closing);
		r.add_tag("sub",rules::opening_and_closing);
		r.add_tag("sup",rules::opening_and_closing);
		r.add_tag("blockquote",rules::opening_and_closing);
		r.add_tag("strong",rules::opening_and_closing);
		r.add_tag("em",rules::opening_and_closing);
		r.add_tag("h1",rules::opening_and_closing);
		r.add_tag("h2",rules::opening_and_closing);
		r.add_tag("h3",rules::opening_and_closing);
		r.add_tag("h4",rules::opening_and_closing);
		r.add_tag("h5",rules::opening_and_closing);
		r.add_tag("h6",rules::opening_and_closing);
		r.add_tag("span",rules::opening_and_closing);
		r.add_tag("code",rules::opening_and_closing);
		r.add_tag("pre",rules::opening_and_closing);
		r.add_property("pre","name",booster::regex("\\w+"));
		r.add_property("pre","class",booster::regex("\\w+"));
		r.add_tag("a",rules::opening_and_closing);
		r.add_uri_property("a","href");
		r.add_tag("hr",rules::stand_alone);
		r.add_tag("br",rules::stand_alone);
		r.add_tag("img",rules::stand_alone);
		r.add_uri_property("img","src");
		r.add_integer_property("img","width");
		r.add_integer_property("img","height");
		r.add_integer_property("img","border");
		r.add_property("img","alt",booster::regex(".*"));
		r.add_tag("table",rules::opening_and_closing);
		r.add_tag("tr",rules::opening_and_closing);
		r.add_tag("th",rules::opening_and_closing);
		r.add_tag("td",rules::opening_and_closing);
		r.add_integer_property("table","cellpadding");
		r.add_integer_property("table","cellspacing");
		r.add_integer_property("table","border");
		r.add_tag("center",rules::opening_and_closing);
		r.add_entity("nbsp");
		r.encoding("UTF-8");
		r.comments_allowed(false);

		initialized = true;
		return r;
	}

	struct init_it { init_it() { xss_filter(); } } instance;


std::string filter(std::string const &html)
{
	return cppcms::xss::filter(html,xss_filter(),cppcms::xss::escape_invalid);
}

std::string markdown_to_html(std::string const &input)
{
	int flags = 0x0004; // no_pants
	/// It is safe to const cast as mkd_string does not 
	/// alter original string
	MMIOT *doc = mkd_string(const_cast<char *>(input.c_str()),input.size(),flags);
	if(!doc) {
		throw std::runtime_error("Failed to read document");
	}

	mkd_compile(doc,flags);
	
	char *content_ptr = 0;
	int content_size = 0;

	content_size = mkd_document(doc,&content_ptr);
	std::string result(content_ptr,content_size);
	mkd_cleanup(doc);
	return result;
}

} // anon


namespace apps {
	void basic_master::clear()
	{
		sql_->close();
	}
	void basic_master::prepare(data::basic_master &c)
	{
		// General information about the blog 
		c.media = media_;
		c.host = host_;
		c.cookie_prefix = cookie_;
		// Common filters
		c.markdown2html = markdown_to_html;
		c.xss = filter;


		if(cache().fetch_data("general_info",c.info))
			return;
		
		cppdb::result r;

		r=sql()<<"SELECT id,value FROM text_options ";

		while(r.next()) {
			std::string id,value;
			r >> id >> value;
			if(id=="blog_title")
				c.info.blog_title = value;
			else if(id == "blog_description")
				c.info.blog_description = value;
			else if(id == "copyright")
				c.info.copyright_string = value;
			else if(id == "contact")
				c.info.contact = value;
		}
		cache().store_data("general_info",c.info);
	}
	basic_master::basic_master(cppcms::service &s) : cppcms::application(s)
	{
		media_ = settings().get<std::string>("blog.media");
		cookie_ = settings().get<std::string>("session.cookies.prefix","cppcms_session");
		host_ = settings().get<std::string>("blog.host");
		conn_str_ = settings().get<std::string>("blog.connection_string");
		sql_.reset(new cppdb::session());
	}
	basic_master::~basic_master() 
	{
	}
	
	cppdb::session &basic_master::sql()
	{
		if(!sql_->is_open())
			sql_->open(conn_str_);
		return *sql_;
	}


}
