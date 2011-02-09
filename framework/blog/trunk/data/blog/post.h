#pragma once
#include <data/blog/master.h>

namespace data {
namespace blog {


	struct category {
		std::string name;
		int id;
	};

	struct post_content {
		int id;
		std::string author;
		std::string title;
		std::string abstract;
		std::string content;
		bool has_content;
		int comment_count;
		time_t published;
		post_content() : 
			id(0),
			has_content(false),
			comment_count(0),
			published(0)
		{
		}
		typedef std::vector<category> categories_type;
		categories_type categories;
	};


	struct comment_form : public cppcms::form {
		cppcms::widgets::text author;
		cppcms::widgets::email mail;
		cppcms::widgets::regex_field url;
		cppcms::widgets::textarea content;
		cppcms::widgets::submit preview;
		cppcms::widgets::submit send;
		cppcms::form inputs;
		cppcms::form buttons;
		comment_form()
		{
			using booster::locale::translate;
			author.non_empty();
			author.id("author");
			author.limits(1,256);
			author.message(translate("Post Comment","Author"));
			mail.non_empty();
			mail.id("email");
			mail.limits(1,256);
			mail.message(translate("Post Comment","E-Mail"));
			mail.help(translate("Post Comment","the email would not displayed"));
			url.regex(cppcms::xss::rules::uri_matcher());
			url.message(translate("Post Comment","URL"));
			url.id("url");
			content.message(translate("Post Comment","Content"));
			content.id("msg");
			content.limits(1,32768);
			preview.value(translate("Post Comment","Preview"));
			send.value(translate("Post Comment","Send"));
			inputs+author+mail+url;
			buttons + send + preview;
			add(inputs);
			add(content);
			add(buttons);
		}
	};

	struct comment {
		std::string username;
		std::string url;
		std::string content;
		int id;
		time_t date;
		comment() : 
			id(0),
			date(0)
		{
		}
	};

	struct post : public master, public post_content {
		typedef std::vector<comment> comments_type;
		comments_type comments;
		std::string preview_content;
		comment_form response;
	};
} // post
} // master
