#include "session_sid.h"
#include "manager.h"
#include "session_bdb_storage.h"
#include "cppcms_error.h"
#include <db_cxx.h>

namespace cppcms {
namespace storage {

class bdb {
	DbEnv *env;
	Db *prim;
	Db *sec;
public:
	static int extracter(Db *sec,const Dbt *key,const Dbt *data, Dbt *res)
	{
		time_t *time=(time_t*)data->get_data();
		res->set_data(time);
		res->set_size(sizeof(time_t));
		return 0;
	}
	static int comp(Db *d,Dbt const *left,Dbt const *right)
	{
		time_t a,b;
		memcpy(&a,left->get_data(),sizeof(time_t));
		memcpy(&b,right->get_data(),sizeof(time_t));
		if(a < b) return -1;
		if(a == b) return 0;
		/*if(a > b)*/ return 1; 
	}
	bdb(string dir,int cache_size=0) :
		env(NULL), prim(NULL), sec(NULL)
	{
		try{
			try{
				env=new DbEnv(0);		
				env->set_cachesize(cache_size/(1<<20),(cache_size % (1<<20)) * 1024,0);
				env->open(dir.c_str(),DB_CREATE | DB_THREAD | DB_INIT_MPOOL,0666);
				prim=new Db(env,0);
				prim->open(NULL,"prim.db",NULL,DB_HASH,DB_THREAD | DB_CREATE,0666);
				sec=new Db(env,0);
				sec->set_flags(DB_DUP);
				sec->set_bt_compare(&bdb::comp);
				sec->open(NULL,"sec.db",NULL,DB_BTREE,DB_THREAD | DB_CREATE,0666);
				prim->associate(NULL,sec,&bdb::extracter,0);
			}
			catch(...){
				if(sec) delete sec;
				if(prim) delete prim;
				if(env) delete env;
				throw;
			}	
		}
		catch(DbException &e)
		{
			if(sec) delete sec;
			if(prim) delete prim;
			if(env) delete env;
			throw cppcms_error(e.what());
		}
	}
	~bdb()
	{
		try {
			sec->close(0);
			prim->close(0);
			env->close(0);
			delete sec;
			delete prim;
			delete env;
		}
		catch(...){}
	}
	void d_store(string const &sid,string const &data,time_t timeout)
	{
		vector<char> vkey(sid.begin(),sid.end());
		Dbt key(&vkey.front(),vkey.size());
		vector<char> record(sizeof(time_t)+data.size(),0);
		*(time_t*)&record.front()=timeout;
		std::copy(data.begin(),data.end(),record.begin()+sizeof(time_t));
		Dbt val(&record.front(),record.size());
		prim->put(NULL,&key,&val,0);
		d_clean();
	}
	bool d_fetch(string const &sid,string &data,time_t *timeout)
	{
		vector<char> vkey(sid.begin(),sid.end());
		Dbt key(&vkey.front(),vkey.size());
		Dbt val;
		val.set_flags(DB_DBT_MALLOC);
		if(prim->get(NULL,&key,&val,0)) {
			return false;
		}
		if(timeout) 
			memcpy(timeout,val.get_data(),sizeof(time_t));
		char *ptr=(char*)val.get_data();
		data.assign(ptr+sizeof(time_t),ptr+val.get_size());
		free(ptr);
		return true;
	}
	void d_del(string const &sid)
	{
		vector<char> vkey(sid.begin(),sid.end());
		Dbt key(&vkey.front(),vkey.size());
		prim->del(NULL,&key,0);
	}
	void d_clean()
	{
		Dbc *cur;
		sec->cursor(NULL,&cur,0);
		time_t now=time(NULL);
		for(;;) {
			time_t timeout;
			Dbt key;
			key.set_data(&timeout);
			key.set_ulen(sizeof(time_t));
			key.set_flags(DB_DBT_USERMEM);
			Dbt val;
			char buf;
			val.set_dlen(1);
			val.set_doff(0);
			val.set_data(&buf);
			val.set_flags(DB_DBT_PARTIAL);
			if(cur->get(&key,&val,DB_FIRST)==0) {
				if(timeout < now)
					cur->del(0);
				else
					break;
			}
			else {
				break;
			}

		}
		cur->close();
	}
	void save(std::string const &sid,time_t timeout,std::string const &in)
	{
		try {
			d_store(sid,in,timeout);
		}
		catch(DbException &e){
			throw cppcms_error(e.what());
		}
	}
	bool load(std::string const &sid,time_t *timeout,std::string &out)
	{
		try {
			return d_fetch(sid,out,timeout);
		}
		catch(DbException &e){
			throw cppcms_error(e.what());
		}
	}
	void remove(std::string const &sid)
	{
		try {
			d_del(sid);
		}
		catch(DbException &e){
			throw cppcms_error(e.what());
		}
	}
};

} // storage

namespace {
struct builder {
	boost::shared_ptr<storage::bdb> db;
	builder(storage::bdb *db_p) : db(db_p) {}
	boost::shared_ptr<session_api> operator()(worker_thread &w)
	{	
		boost::shared_ptr<session_server_storage> storage(new session_bdb_storage(db));
		return boost::shared_ptr<session_api>(new session_sid(storage));
	}
};
}

session_backend_factory session_bdb_storage::factory(manager &app)
{
	string dbdir=app.config.sval("session.bdb_dir");
	int memsize=app.config.ival("session.bdb_cache_size",4);
	return builder(new storage::bdb(dbdir,memsize));
}

session_bdb_storage::session_bdb_storage(boost::shared_ptr<storage::bdb> db_):
	db(db_)
{
}
void session_bdb_storage::save(std::string const &sid,time_t timeout,std::string const &in)
{
	db->save(sid,timeout,in);
}
bool session_bdb_storage::load(std::string const &sid,time_t *timeout,std::string &out)
{
	return db->load(sid,timeout,out);
}
void session_bdb_storage::remove(std::string const &sid) 
{
	return db->remove(sid);
}

} // cppcms
