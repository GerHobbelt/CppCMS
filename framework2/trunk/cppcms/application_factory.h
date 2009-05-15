#ifndef CPPCMS_APPLICATION_FACTORY_H
#define CPPCMS_APPLICATION_FACTORY_H

#include <cppcms/defs.h>
#include <memory>

namespace cppcms {

	class application;

	class CPPCMS_API application_factory {
	public:
		virtual std::auto_ptr<application> operator()() const = 0;
		virtual ~application_factory();
	};

	namespace app { namespace details {
		template<typename T>
		class factory0 {
		public:
			virtual std::auto_ptr<application> operator()() const
			{
				return std::autp_ptr<application>(new T());
			};
		};
		template<typename T,typename P1>
		class factory1 {
			P1 p1;
		public:
			factory1(P1 _p1) : p1(_p1) {}
			virtual std::auto_ptr<application> operator()() const
			{
				return std::autp_ptr<application>(new T(P p1));
			};
		};
		template<typename T,typename P1,typename P2>
		class factory2 {
			P1 p1;
			P2 p2;
		public:
			factory1(P1 _p1,P2 _p2) : p1(_p1),p2(_p2) {}
			virtual std::auto_ptr<application> operator()() const
			{
				return std::autp_ptr<application>(new T(p1,p2));
			};
		};
		template<typename T,typename P1,typename P2,typename P3>
		class factory3 {
			P1 p1;
			P2 p2;
			P3 p3;
		public:
			factory1(P1 _p1,P2 _p2,P3 _p3) : p1(_p1),p2(_p2),p3(_p3) {}
			virtual std::auto_ptr<application> operator()() const
			{
				return std::autp_ptr<application>(new T(p1,p2,p3));
			};
		};
	
	} } // app::details
		
	template<typename T>
	std::auto_ptr<application_factory> simple_application_factory()
	{
		return std::autp_ptr<application_factory>(new app::details::factory0<T>());
	}
	template<typename T,typename P1>
	std::auto_ptr<application_factory> simple_application_factory(P1 p1)
	{
		return std::autp_ptr<application_factory>(new app::details::factory1<T,P1>(p1));
	}
	template<typename T,typename P1,typename P2>
	std::auto_ptr<application_factory> simple_application_factory(P1 p1,P2 p2)
	{
		return std::autp_ptr<application_factory>(new app::details::factory2<T,P1,P2>(p1,p2));
	}
	template<typename T,typename P1,typename P2,typename P3>
	std::auto_ptr<application_factory> simple_application_factory(P1 p1,P2 p2,P3 p3)
	{
		return std::autp_ptr<application_factory>(new app::details::factory3<T,P1,P2,P3>(p1,p2,p3));
	}

} // cppcms

#endif
