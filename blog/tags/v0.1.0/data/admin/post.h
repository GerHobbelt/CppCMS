#pragma once
#include <data/admin/master.h>

namespace data {
	namespace admin {
		struct post : public master {
			struct post_form : public cppcms::form {
				
				cppcms::widgets::text title;
				cppcms::widgets::textarea abstract;
				cppcms::widgets::textarea content;

				cppcms::widgets::submit save; 
				cppcms::widgets::submit save_and_continue; 
				cppcms::widgets::submit change_status; 
				cppcms::widgets::submit remove; 
				cppcms::widgets::checkbox really_remove; 

				std::vector<cppcms::widgets::checkbox *> add_to_cat_list;
				std::vector<cppcms::widgets::checkbox *> del_from_cat_list;

				cppcms::form inputs;
				cppcms::form buttons;
				cppcms::form add_to_cat;
				cppcms::form del_from_cat;

				post_form()
				{
					title.message(booster::locale::translate("Title"));
					title.non_empty();
					title.size(80);
					abstract.message(booster::locale::translate("Abstract"));
					abstract.non_empty();
					abstract.rows(30);
					abstract.cols(80);
					content.message(booster::locale::translate("Content"));
					content.rows(30);
					content.cols(80);
					save.value(booster::locale::translate("Save"));
					save_and_continue.value(booster::locale::translate("Save and Continue"));
					remove.value(booster::locale::translate("Delete"));
					really_remove.message(booster::locale::translate("Confirm delete"));
					really_remove.error_message(booster::locale::translate("Check this to delete post"));
					inputs.add(title);
					inputs.add(abstract);
					inputs.add(content);
					buttons.add(save);
					buttons.add(save_and_continue);
					buttons.add(change_status);
					buttons.add(remove);
					buttons.add(really_remove);
					add(inputs);
					add(buttons);
					add(add_to_cat);
					add(del_from_cat);
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
			post_form form;
			post() : id(0) {}
		};
	}
}
