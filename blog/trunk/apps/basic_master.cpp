#include <apps/basic_master.h>
#include <apps/dbversion.h>
#include <data/basic_master.h>
#include <cppdb/frontend.h>
#include <cppcms/json.h>
#include <cppcms/urandom.h>
#include <cppcms/xss.h>
#include <booster/nowide/fstream.h>
#include <booster/nowide/cstdio.h>
#include <booster/log.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <unistd.h>

extern "C" {
	#include <mkdio.h>
}


namespace {
	cppcms::xss::rules const &xss_filter()
	{
		static cppcms::xss::rules r;
		static bool initialized = false;
		if(initialized)
			return r;
		using namespace cppcms::xss;

		r.html(rules::xhtml_input);
		r.add_tag("ol",rules::opening_and_closing);
		r.add_tag("ul",rules::opening_and_closing);
		r.add_tag("li",rules::opening_and_closing);
		r.add_tag("p",rules::opening_and_closing);
		r.add_tag("b",rules::opening_and_closing);
		r.add_tag("i",rules::opening_and_closing);
		r.add_tag("tt",rules::opening_and_closing);
		r.add_tag("sub",rules::opening_and_closing);
		r.add_tag("sup",rules::opening_and_closing);
		r.add_tag("blockquote",rules::opening_and_closing);
		r.add_tag("strong",rules::opening_and_closing);
		r.add_tag("em",rules::opening_and_closing);
		r.add_tag("h1",rules::opening_and_closing);
		r.add_tag("h2",rules::opening_and_closing);
		r.add_tag("h3",rules::opening_and_closing);
		r.add_tag("h4",rules::opening_and_closing);
		r.add_tag("h5",rules::opening_and_closing);
		r.add_tag("h6",rules::opening_and_closing);
		r.add_tag("span",rules::opening_and_closing);
		r.add_tag("code",rules::opening_and_closing);
		r.add_tag("pre",rules::opening_and_closing);
		r.add_property("pre","name",booster::regex("\\w+"));
		r.add_property("pre","class",booster::regex("\\w+"));
		r.add_tag("a",rules::opening_and_closing);
		r.add_uri_property("a","href");
		r.add_tag("hr",rules::stand_alone);
		r.add_tag("br",rules::stand_alone);
		r.add_tag("img",rules::stand_alone);
		r.add_uri_property("img","src");
		r.add_integer_property("img","width");
		r.add_integer_property("img","height");
		r.add_integer_property("img","border");
		r.add_property("img","alt",booster::regex(".*"));
		r.add_tag("table",rules::opening_and_closing);
		r.add_tag("tr",rules::opening_and_closing);
		r.add_tag("th",rules::opening_and_closing);
		r.add_tag("td",rules::opening_and_closing);
		r.add_integer_property("table","cellpadding");
		r.add_integer_property("table","cellspacing");
		r.add_integer_property("table","border");
		r.add_tag("center",rules::opening_and_closing);
		r.add_entity("nbsp");
		r.encoding("UTF-8");
		r.comments_allowed(false);

		initialized = true;
		return r;
	}

	struct init_it { init_it() { xss_filter(); } } instance;


std::string filter(std::string const &html)
{
	return cppcms::xss::filter(html,xss_filter(),cppcms::xss::escape_invalid);
}

std::string markdown_to_html(std::string const &input)
{
	int flags = 0x0004; // no_pants
	/// It is safe to const cast as mkd_string does not 
	/// alter original string
	MMIOT *doc = mkd_string(const_cast<char *>(input.c_str()),input.size(),flags);
	if(!doc) {
		throw std::runtime_error("Failed to read document");
	}

	mkd_compile(doc,flags);
	
	char *content_ptr = 0;
	int content_size = 0;

	content_size = mkd_document(doc,&content_ptr);
	std::string result(content_ptr,content_size);
	mkd_cleanup(doc);
	return result;
}

class tex_to_gif {
public:
	tex_to_gif(cppcms::json::value const &config)
	{
		path_to_latex_ = config.get("blog.tex.latex","/usr/bin/latex");
		path_to_dvigif_ = config.get("blog.tex.dvigif","/usr/bin/dvigif");
		path_to_tmp_ = config.get("blog.tex.temp_dir","/tmp");
		std::string tmp_www_dir = config.get<std::string>("blog.media") + "/tex";
		www_dir_ = config.get("blog.tex.www_root",tmp_www_dir);
		output_dir_ = config.get("blog.tex.output_dir","");
		if(output_dir_.empty())
			output_dir_ = 	config.get<std::string>("file_server.document_root",".") 
					+ "/" + www_dir_; 
	}
	~tex_to_gif()
	{
	}
	std::string convert(std::string const &input) const
	{
		std::string name = cppcms::util::md5hex(input);
		std::string final;
		final.reserve(64);
		final+= "/";
		final+=name;
		final+=".gif";
		std::string www_name = www_dir_ + final;
		std::string gif_name = output_dir_ + final;
		struct stat st;
		if(:: stat(gif_name.c_str(),&st) == 0) {
			return www_name;
		}
		unsigned char buf[8];
		cppcms::urandom_device rnd;
		rnd.generate(buf,sizeof(buf));
		char tmp_name[sizeof(buf) * 2 + 1];
		for(unsigned i=0;i<sizeof(buf);i++)
			snprintf(tmp_name + i*2,3,"%02x",buf[i]);
		if(create_gif(input,tmp_name,gif_name))
			return www_name;
		return std::string(); 
	}
private:

	bool os_spawn(char const *dir,std::string const &program,char const *params[]) const
	{
		int pid = fork();
		if(pid < 0) {
			BOOSTER_ERROR("blog") << "Fork failed:" << strerror(errno);
			return false;
		}
		if(pid == 0) {
			FILE *f;
			f=freopen("/dev/null","r",stdin); (void)(f);
			f=freopen("/dev/null","w",stdout);(void)(f);
			f=freopen("/dev/null","w",stderr);(void)(f);
			if(dir) {
				int r = chdir(dir);
				int err = errno;
				if(r < 0) {
					BOOSTER_ERROR("blog") << "Failed to change directory to " << dir << ":" << strerror(err);
					exit(1);
				}
			}
			execv(program.c_str(),const_cast<char**>(params));
			int err = errno;
			BOOSTER_ERROR("blog") << "Failed to execute " << program << ":" << strerror(err);
			exit(1);
		}
		else {
			int status=1,r;
			do {
				r = waitpid(pid,&status,0);
			} while(r < 0 && errno == EINTR);
			if(r < 0)
				return false;
			if(!WIFEXITED(status))
				return false;
			bool result = WEXITSTATUS(status) == 0;
			if(!result) {
				BOOSTER_ERROR("blog") << "Process " << program << " exited with status " <<WEXITSTATUS(status);
			}
			return result;

		}
	}

	bool spawn(char const *dir,std::string const &program,std::string const &param) const
	{
		char const *params[3] = {program.c_str(),param.c_str() };
		return os_spawn(dir,program,params);
	}

	bool spawn(char const *dir,std::string const &program,std::string const &param1,std::string const &param2) const
	{
		char const *params[4] = {program.c_str(),param1.c_str(),param2.c_str() };
		return os_spawn(dir,program,params);
	}

	bool spawn(char const *dir,std::string const &program,std::string const &param1,std::string const &param2,std::string const &param3) const
	{
		char const *params[5] = {program.c_str(),param1.c_str(),param2.c_str(),param3.c_str() };
		return os_spawn(dir,program,params);
	}


	bool create_gif(std::string const &tex,std::string const &random_name,std::string const &output) const
	{
		std::string fname = path_to_tmp_ + "/" + random_name;
		struct scoped_clean {
			std::string name;
			scoped_clean(std::string const &n) : name(n) {}
			~scoped_clean() {
				using namespace booster::nowide;
				remove((name+".tex").c_str());
				remove((name+".aux").c_str());
				remove((name+".log").c_str());
				remove((name+".gif").c_str());
				remove((name+".dvi").c_str());
			}
		} scope(fname);
		std::string tex_file = fname + ".tex";
		std::ofstream texsrc(tex_file.c_str());
		if(!texsrc) {
			return false;
		}
		texsrc << 
			"\\documentclass[12pt]{article}\n"
			"\\usepackage[latin1]{inputenc}\n"
			"\\usepackage{amsmath}\n"
			"\\usepackage{amsfonts}\n"
			"\\usepackage{amssymb}\n"
			"\\pagestyle{empty}\n"
			"\\setlength{\\oddsidemargin}{-5in}\n"
			"\\setlength{\\topmargin}{0cm}\n"
			"\\setlength{\\headheight}{-5in}\n"
			"\\setlength{\\headsep}{0cm}\n"
			"\\begin{document}\n"
			"\\[" << tex << "\\]\n"
			"\\end{document}\n" << std::flush;

		texsrc.close();

		if(!spawn(path_to_tmp_.c_str(),path_to_latex_,"--interaction=nonstopmode",tex_file)) {
			return false;
		}

		std::string gif_name = fname + ".gif";
		if(!spawn(0,path_to_dvigif_,fname + ".dvi", "-o" , gif_name)){
			return false;
		}

		if(booster::nowide::rename(gif_name.c_str(),output.c_str()) != 0){
			char buf[1024];
			booster::nowide::ifstream in_file(gif_name.c_str(),std::fstream::binary);
			if(!in_file)
				return false;
			booster::nowide::ofstream out_file(output.c_str(),std::fstream::binary);
			if(!out_file)
				return false;
			while(in_file.good() && !in_file.eof()) {
				in_file.read(buf,sizeof(buf));
				out_file.write(buf,in_file.gcount());
			}
			in_file.close();
			out_file.close();
			if(out_file.fail()) {
				booster::nowide::remove(output.c_str());
				return false;
			}
			return  true;
		}
		return true;
	}

	std::string path_to_latex_;
	std::string path_to_dvigif_;
	std::string path_to_tmp_;
	std::string output_dir_;
	std::string www_dir_;

}; // tex_to_gif

std::auto_ptr<tex_to_gif> tex_filter;

std::string latex_filter(std::string const &in)
{
	std::string out;
	if(!tex_filter.get()) {
		out = in;
		return out;
	}
	out.reserve(in.size());
	size_t p_old=0,p1=0,p2=0;
	while((p1=in.find("[tex]",p_old))!=std::string::npos && (p2=in.find("[/tex]",p_old))!=std::string::npos && p2>p1) {
		out.append(in,p_old,p1-p_old);
		p1+=5;
		std::string tex(in,p1,p2-p1);
		std::string wwwfile = tex_filter->convert(tex);
		std::string html_tex = cppcms::util::escape(tex);
		if(wwwfile.empty())  {
			out+=html_tex;
		}
		else {
			out+="<img src='";
			out+= wwwfile;
			out+="' alt='";
			out+=html_tex;
			out+="' align='absmiddle' />";
		}
		p_old=p2+6;
	}
	out.append(in,p_old,in.size()-p_old);
	return out;
}


} // anon


namespace apps {
	namespace {
		typedef bool (*upgrade_callback)(cppdb::session &sql);

		bool dummy_upgrade(cppdb::session &)
		{
			throw database_version_error();
		}

		bool upgrade_2_3(cppdb::session &sql)
		{
			BOOSTER_INFO("blog") << "Upgrading 2 to 3";
			sql<<	"UPDATE post2cat "
				"SET is_open = "
				"   ( "
				"     SELECT posts.is_open "
				"     FROM posts "
				"     WHERE posts.id = post2cat.post_id "
				"   ) " 
				<< cppdb::exec;
			sql<<	"UPDATE text_options "
				"SET value = '3' "
				"WHERE id='dbversion' "
				<< cppdb::exec;

		 	return true;
		}

		bool upgrade(std::string const &current_ver,cppdb::session &sql)
		{
			int ver=atoi(current_ver.c_str());
			int final_ver = atoi(CPPBLOG_DBVERSION);
			BOOSTER_INFO("blog") << "Upgrading datavase from " << current_ver << " to " <<
						final_ver;
			if(ver > final_ver)
				return false;
			if(ver < 2)
				return false;
			static const upgrade_callback callbacks[] = {
				dummy_upgrade,
				dummy_upgrade, // 0,1 not supported
				upgrade_2_3
			};
			cppdb::transaction tr(sql);
			while(ver < final_ver) {

				upgrade_callback call=callbacks[ver];

				if(!call(sql))
					return false;
				ver ++;
			}
			BOOSTER_INFO("blog") << "Upgrade completed";
			tr.commit();
			return true;
		}
	}
	void init_tex_filer(cppcms::json::value const &s)
	{
		if(s.get("blog.tex.enable",false)) {
			tex_filter.reset(new tex_to_gif(s));
		}
	}
	void basic_master::clear()
	{
		sql_->close();
	}
	void basic_master::prepare(data::basic_master &c)
	{
		// General information about the blog 
		c.media = media_;
		c.host = host_;
		c.cookie_prefix = cookie_;
		// Common filters
		c.markdown2html = markdown_to_html;
		c.xss = filter;
		c.latex = latex_filter;


		if(cache().fetch_data("options",c.info))
			return;
		
		cppdb::result r;

		r=sql()<<"SELECT id,value FROM text_options ";

		std::string dbversion;
		std::string is_configured;

		//
		// Update me when needed!
		//
		char const *expected_database_version = CPPBLOG_DBVERSION;

		while(r.next()) {
			std::string id,value;
			r >> id >> value;
			if(id=="blog_title")
				c.info.blog_title = value;
			else if(id == "blog_description")
				c.info.blog_description = value;
			else if(id == "copyright")
				c.info.copyright_string = value;
			else if(id == "contact")
				c.info.contact = value;
			else if(id == "dbversion")
				dbversion=value;
			else if(id == "is_configured")
				is_configured = value;
		}
		if(dbversion!=expected_database_version && !upgrade(dbversion,sql())) {
			throw database_version_error();
		}
		if(is_configured!="yes") {
			throw database_is_not_configured_error();
		}
		cache().store_data("options",c.info);
	}
	basic_master::basic_master(cppcms::service &s) : cppcms::application(s)
	{
		media_ = settings().get<std::string>("blog.media");
		cookie_ = settings().get<std::string>("session.cookies.prefix","cppcms_session");
		host_ = settings().get<std::string>("blog.host");
		conn_str_ = settings().get<std::string>("blog.connection_string");
		sql_.reset(new cppdb::session());
	}
	basic_master::~basic_master() 
	{
	}
	
	cppdb::session &basic_master::sql()
	{
		if(!sql_->is_open())
			sql_->open(conn_str_);
		return *sql_;
	}


}
