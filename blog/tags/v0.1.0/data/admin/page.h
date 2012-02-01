#pragma once
#include <data/admin/master.h>

namespace data {
	namespace admin {
		struct page : public master {
			struct page_form : public cppcms::form {
				cppcms::widgets::text title;
				cppcms::widgets::textarea content;
				cppcms::widgets::submit save; 
				cppcms::widgets::submit save_and_continue; 
				cppcms::widgets::submit change_status; 
				cppcms::widgets::submit remove; 
				cppcms::widgets::checkbox really_remove; 

				cppcms::form inputs;
				cppcms::form buttons;

				page_form()
				{
					title.message(booster::locale::translate("Title"));
					title.non_empty();
					content.message(booster::locale::translate("Content"));
					content.non_empty();
					content.rows(30);
					content.cols(80);
					save.value(booster::locale::translate("Save"));
					save_and_continue.value(booster::locale::translate("Save and Continue"));
					remove.value(booster::locale::translate("Delete"));
					really_remove.message(booster::locale::translate("Confirm delete"));
					really_remove.error_message(booster::locale::translate("Check this to delete page"));
					inputs.add(title);
					inputs.add(content);
					buttons.add(save);
					buttons.add(save_and_continue);
					buttons.add(change_status);
					buttons.add(remove);
					buttons.add(really_remove);
					add(inputs);
					add(buttons);
				}
				virtual bool validate()
				{
					if(!cppcms::form::validate())
						return false;
					if(remove.value() && !really_remove.value()){
						really_remove.valid(false);
						return false;
					}
					return true;
				}
			};

			int id;
			page_form form;
			page() : id(0) {}
		};
	}
}
