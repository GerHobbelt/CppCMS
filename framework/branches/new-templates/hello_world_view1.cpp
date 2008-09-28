#include "hello_world_view.h"

namespace view1 {
	struct master: public cppcms::base_view {
		view::master &content;
		master(cppcms::worker_thread *w,view::master &c) : cppcms::base_view(w) , content(c)
		{
		}
		virtual void render() {
			cout<<"<html>";
			cout<<"<title>"<<content.title<<"</title>\n";
			cout<<"<body>\n";
			body();
			cout<<"</body>\n";
			cout<<"</html>\n";
		};
		virtual void body(){};
	};
	struct hello: public master {
		view::hello &content;
		hello(cppcms::worker_thread *w,view::hello &c) : master(w,c) , content(c) {};
		virtual void body() 
		{
			cout<<"<h1>"<<content.title<<"</h1>";
			cout<<"<p>"<<escape(content.msg)<<"</p>";
		};
	};
};

namespace {
	struct loader {
		loader() {
			using namespace cppcms::details;
			views_storage &vs=views_storage::instance();
			vs.add_view("view1","hello",view_builder<view1::hello,view::hello>());
			vs.add_view("view1","master",view_builder<view1::master,view::master>());
		};
		~loader() {
			using namespace cppcms::details;
			views_storage &vs=views_storage::instance();
			vs.remove_views("view1");
		};

	} entry;
} // Empty namespace

