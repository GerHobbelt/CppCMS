#ifndef CPPCMS_URL_DISPATCHER_H
#define CPPCMS_URL_DISPATCHER_H

#include <cppcms/util/noncopyable.h>
#include <cppcms/util/callback.h>
#include <string>

namespace cppcms {
	namespace util {
		class regex;
	}

	class application;

	class url_dispatcher_impl;
	class CPPCMS_API url_dispatcher : public util::noncopyable {
		std::auto_ptr<url_dispatcher_impl> pimpl;
	public:
		// Handlers 
		typedef util::callback0 handler;
		typedef util::callback1<std::string> handler1;
		typedef util::callback2<std::string,std::string> handler2;
		typedef util::callback3<std::string,std::string,std::string> handler3;
		typedef util::callback4<std::string,std::string,std::string,std::string> handler4;
		typedef util::callback5<std::string,std::string,std::string,std::string,std::string> handler5;
		typedef util::callback6<std::string,std::string,std::string,std::string,std::string,std::string> handler6;

		// Mounting other application
		void mount(std::string prefix,application &);
		void mount(util::regex const &match,application &,int select);

		void assign(util::regex const &match,handler);
		void assign(util::regex const &match,handler1,int exp1);
		void assign(util::regex const &match,handler2,int exp1,int exp2);
		void assign(util::regex const &match,handler3,int exp1,int exp2,int exp3);
		void assign(util::regex const &match,handler4,int exp1,int exp2,int exp3,int exp4);
		void assign(util::regex const &match,handler5,int exp1,int exp2,int exp3,int exp4,int exp5);
		void assign(util::regex const &match,handler6,int exp1,int exp2,int exp3,int exp4,int exp5,int exp6);

		bool matches(std::string path);
		bool dispatch(std::string path);

		url_dispatcher();
		~url_dispatcher();
	};

} // cppcms

#endif
