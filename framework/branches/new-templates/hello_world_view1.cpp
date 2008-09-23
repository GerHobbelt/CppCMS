#include "hello_world_view.h"

namespace view1 {
	struct master : virtual public view::master {
		virtual void render() {
			cout()<<"<html><body>\n";
			content();
			cout()<<"</body></html>\n";
		}
	};
	struct hello: virtual public view::hello, virtual public master {};
};

namespace view2 {
	struct master : virtual public view::master {
		virtual void render() {
			cout()<<"<html><body><h1>\n";
			content();
			cout()<<"</h1></body></html>\n";
		}
	};
	struct hello: virtual public view::hello, virtual public master {
		virtual void content() {
			cout() << "Good Bye World\n";
		};
	};
};

namespace {
	struct loader {
		loader() {
			using namespace cppcms::details;
			views_storage &vs=views_storage::instance();
			vs.add_view("view1",typeid(view::hello).name(),view_builder<view1::hello>());
			vs.add_view("view1",typeid(view::master).name(),view_builder<view1::hello>());
			vs.add_view("view2",typeid(view::hello).name(),view_builder<view2::hello>());
			vs.add_view("view2",typeid(view::master).name(),view_builder<view2::hello>());
		};
		~loader() {
			using namespace cppcms::details;
			views_storage &vs=views_storage::instance();
			vs.remove_views("view1");
			vs.remove_views("view2");
		};

	} entry;
} // Empty namespace

