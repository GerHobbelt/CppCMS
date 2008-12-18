namespace cppcms {

namespace storage {

#define LOCK_SIZE 257

void io::close(int fid)
{
	::close(fid);
}

int io::lock_id(std::string const &sid) const
{
	int id;
	char buf[3] = {0};
	buf[0]=sid.at(0);
	buf[1]=sid.at(1);
	sscnaf(buf,"%x",&id);
	return id;
};
string io::mkname(std::string const &sid) const
{
	return dir+"/"+sid;
}
void io::write(std::string const &sid,time_t timestamp,void const *buf,size_t len) const
{
	string name=mkname(sid);
	int fid=::open(name.c_str(),O_CREAT | O_TRUNK | O_WRONLY ,0666);
	if(fid<0) { throw cppcms_error(errno,"storage::local_id"); }
	::write(fid,&timestamp,sizeof(time_t));
	::write(fid,buf,len);
	unsigned char crc=boost::crc<8>(buf,len);
	::write(fid,&crc,1);
	close(fid);
}
bool io::read(std::string const &sid,time_t &timestamp,vector<unsigned char *> *out) const 
{
	int fid=-1;
	string name=mkname(sid);
	try {
		fid=::open(name.c_str(),O_RDONLY);
		if(fid<0) {
			return false;
		}
		time_t tmp;
		if(::read(fid,&tmp,sizeof(time_t))!=sizeof(time_t) || tmp < time(NULL)) 
			goto error_exit;
		timestamp=tmp;
		if(!out) {
			close(fid);
			return true;
		}
		int size=lseek(fid,0,SEEK_END);
		if(size==-1 || size <sizeof(time_t)+1) 
			goto error_exit;
		if(lseek(fid,sizeof(time_t),SEEK_SET) < 0) 
			goto error_exit;
		out->resize(size-1,0);
		if(::read(fid,&out->front(),size-1)!=size-1) 
			goto error_exit;
		unsigned char crc_ch,crc=boost::crc<8>(&out->front(),size-1);
		if(::read(fid,&crc_ch,1)!=1 || crc_ch != crc)
			goto error_exit;
		close(fid);
		return true;
	error_exit:
		close(fid);
		::unlink(name.c_str());
		return false;
	}
	catch(...) {
		if(fid>=0) close(fid);
		::unlink(name.c_str());
		throw;
	}
}
void io::unlink(std::string const &sid) const 
{
	string name=mkname(sid);
	::unlink(name.c_str());
}

local_io::local_io(std::string dir,pthread_rwlock_t *l):
	io(dir),
	locks(l)
{
}

void local_io::wrlock(std::string const &sid) const
{
	pthread_rwlock_wrlock(&locks[lock_id[sid]]);
}
void local_io::rdlock(std::string const &sid) const
{
	pthread_rwlock_rdlock(&locks[lock_id[sid]]);
}
void local_io::unlock(std::string sid) const
{
	pthread_rwlock_unlock(&locks[lock_id[sid]]);
}

void nfs_io::close(int fid)
{
	fsync(fid);
	close(fid);
}

nfs_io::nfs_io(std::string dir) : io(dir)
{
	fid=::open(dir+"/"+"nfs.lock",O_CREAT | O_RDWR);
	if(fid<0) {
		throw cppcms_error(errno,"storage::nfs_io::open");
	}
	::lseek(fid,LOCK_SIZE,SEEK_SET);
	::write(fid,"",1);
}

nfs_io::~nfs_io()
{
	close(fid);
}

namespace {

bool flock(int fid,int how,int pos)
{
	struct flock lock;
	memset(lock,0,sizeof(lock));
	lock.l_type=how;
	lock.l_whence=SEEK_SET;
	lock.l_start=pos;
	lock.l_len=1;
	int res;
	while((res=fcntl(fid,F_SETLKW,&lock)!=0) && errno==EINTR)
		;
	if(res) return false;
	return true;	
}

void nfs_io::wrlock(std::string const &sid) const
{
	if(!flock(fid,F_WRLCK,lock_id(sid)))
		throw cppcms_error(errno,"storage::nfs_io::fcntl::WRITE LOCK");
}
void nfs_io::rdlock(std::string const &sid) const
{
	if(!flock(fid,F_RDLCK,lock_id(sid)))
		throw cppcms_error(errno,"storage::nfs_io::fcntl::READ LOCK");
}
void nfs_io::unlock(std::string sid) const
{
	flock(fid,F_UNLCK,lock_id(sid));
}

} // anon namespace

pthread_rwlock_t *thread_io::create_locks()
{
	pthread_rwlock_t *array = new pthread_rwlock_t [LOCK_SIZE];
	for(int i=0;i<LOCK_SIZE;i++) {
		if(pthread_rwlock_init(array+i,NULL)) {
			int err=errno;
			i--;
			for(;i>=0;i--) {
				pthread_rwlock_destroy(array+i);
			}
			delete [] array;
			throw cppcms_error(err,"storage:pthread_rwlock_init");
		}
	}
	return array;
}
thread_io::thread_io(std::string dir) :
	local_io(dir,create_locks())
{
}
thread_io::~thread_io()
{
	for(i=0;i<LOCK_SIZE;i++) {
		pthread_rwlock_destroy(locks+i);
	}
	delete [] locks;
}

pthread_rwlock_t *shmem_io::create_locks()
{
	int size=sizeof(pthread_rwlock_t)*LOCK_SIZE;
	void *ptr=mmap(0,size,PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, 0,0);
	if(ptr==MAP_FAILED) {
		throw cppcms_error(errno,"storage:mmap");
	}
	pthread_rwlock_t *array =(pthread_rwlock_t*)ptr;
	for(int i=0;i<LOCK_SIZE;i++) {
		pthread_rwlockattr_t attr;
		if(	pthread_rwlockattr_init(&attr) != 0 
			|| pthewad_rwlockattr_setpshared(&attr,PTHREAD_PROCESS_SHARED) != 0
			|| pthread_rwlock_init(array+i,&attr) !=0)
		{
			int err=errno;
			i--;
			for(;i>=0;i--) {
				pthread_rwlock_destroy(array+i);
			}
			munmap(ptr,size);
			throw cppcms_error(err,"storage:pthread_rwlock_init");
		}
		pthread_rwlockattr_destroy(&attr);
	}
	creator_pid=getpid();
	return array;
}
shmem_io::shmem_io(std::string dir) :
	local_io(dir,create_locks())
{
}
thread_io::~thread_io()
{
	if(creator_pid==getpid()) {
		for(i=0;i<LOCK_SIZE;i++) {
			pthread_rwlock_destroy(locks+i);
		}
	}
	munmap(locks,sizeof(*locks)*LOCK_SIZE);
}



} // namespace storeage



void session_file_storage::save(string const &sid,time_t timeout,string const &in)
{
	try {
		io->wrlock(sid);
		io->write(sid,timeout,in.data(),in.size());
		io->unlock(sid);
	}
	catch(...) {
		io->unlock(sid);
		throw;
	}
}

bool session_file_storage::load(string const &sid,time_t *timeout,string &out)
{
	try {
		io->rdlock(sid);
		vector<unsigned char> data;
		time_t tmp;
		bool res=io->read(sid,tmp,data);
		io->unlock(sid);
		if(timeout) *timeout=tmp;
		out.assign(data.begin(),data.end());
		return res;
	}
	catch(...) {
		io->unlock(sid);
		throw;
	}
}

void session_file_storage::remove(string const &sid)
{
	try {
		io->wrlock(sid);
		io->unlink(sid);
		io->unlock(sid);
	}
	catch(...) {
		io->unlock(sid);
		throw;
	}
	
}

void session_file_storage::gc()
{
	DIR *d=NULL;
	try{
		string dir=io->get_dir();
		d=opendir(dir.c_str());
		struct dirent entry,*entry_p;
		while(readdir_r(d,&entry,&entry_p)==0) {
			for(i=0;i<32;i++) {
				if(!isxdigit(entry.d_name[i]))
					break;
			}
			if(i!=32 || entry.d_name[i]!=0) 
				continue;
			string sid=entry.d_name;
			try{
				io->rdlock(sid);
				time_t tmp;
				io->read(sid,tmp,NULL);
				// if it timeouted -- will be removed
				io->unlock(sid);
			}
			catch(...) {
				io->unlock(sid);
				throw;
			}
			
		}
		closedir(d);
	}
	catch(...) {
		if(d) closedir(d);
		throw;
	}
}


} // namespace cppcms
