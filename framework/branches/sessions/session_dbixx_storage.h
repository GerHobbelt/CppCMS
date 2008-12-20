#ifndef CPPCMS_DBIXX_STORAGE_H
#define CPPCMS_DBIXX_STORAGE_H

#include "session_storage.h"
#include <dbixx/dbixx.h>

namespace cppcms {
class session_dbixx_storage : public session_server_storage {
	dbixx::session &sql;
public:
	session_dbixx_storage(dbixx::session &sql_) :
		sql(sql_)
	{
	}
protected:	
	virtual void save(std::string const &sid,time_t timeout,std::string const &in) = 0;
	{
		dbixx::transaction tr(sql);
		remove(sid);
		std::tm t;
		localtime_r(&timeout,&t);
		sql<<"INSERT INTO cppcms_sessions(sid,timeout,data) "
		     "VALUES (?,?,?)",sid,t,in;
		sql.exec();
		tr.commit();
		time(&now);
		localtime_r(&now,&t);
		sql<<"DELETE FROM cppcms_sessions WHERE timeout < ?",t;
		sql.exec();
	}

	virtual bool load(std::string const &sid,time_t *timeout,std::string &out) = 0;
	{
		sql<<"SELECT timeout,data FROM cppcms_sessions WHERE sid=?",sid;
		dbixx::row r;
		if(sql.single(r)) {
			std::tm t;
			std::string data;
			r>>t>>data;
			time_t tmp=mktime(&t);
			if(tmp<time(NULL))
				return false;
			if(*timeout) *timeout=tmp;
			out.swap(data);
			return true;
		}
		return false;
	}
	virtual void remove(std::string const &sid)
	{
		sql<<"DELETE FROM cppcms_sessions WHERE sid=?",sid;
		sql.exec();
	}
};


} // cppcms

#endif
