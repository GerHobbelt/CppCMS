{
	
	"blog" : {
		// The location of the cppblog media files
		// on the web server
		"media" : "/media",
		// The script name for the blog,
		// if it runs in the root of the web server
		// it should be ""
		"root" : "/blog",
		// The host the blog runs on. it is important for RSS feeds.
		"host" : "www.example.com",

		//
		// CppDB connection string
		//
		//"connection_string" : "sqlite3:db=cppcms.db;@pool_size=10",
		//"connection_string" : "mysql:database=blog;user=root;password=root;@pool_size=10",
		//"connection_string" : "postgresql:dbname=blog;@pool_size=10",
		//

		"tex" : {
			// Set it to true to emab
			"enable" : true,
			// Path to the latex executable
			// "latex" : "/usr/bin/latex",

			// Path to dvigif converter
			// "dvigif" : "/usr/bin/dvigif",

			// Temporary directory for temporary files
			// "temp_dir" : "/tmp",

			// Location of the final gif files, note it should
			// be under media directory (see blog.media) ,
			//  make sure it is writeable by the web server
			// "output_dir" : "/var/www/media/tex"
		}
	},
	"service" : {
		"api" : "fastcgi",
		"socket" : "stdin"
	},

	"views" : {
		 "default_skin" : "orangesky" ,
		 "paths" : [ "/usr/local/lib/cppblog" ],
		 "skins" : [ "contendend" , "orangesky" ],
	},
	"session" : {
		"expire" : "renew",
		"timeout" : 604800, // a week
		"location" : "both",
		"client" : {
			"hmac" : "sha1",
			// "hmac_key" : "" // Replace Me
		},
		"server" : {
			"storage" : "files",
			// Change it for your specific application
			"dir" : "/tmp/cppblog_dir"
		}
	},
	"cache" : {
		"backend" : "thread_shared",
		"limit" : 100,
	},
	"localization" : {
		"messages" : {
			"paths" : [ "/usr/local/share/locale" ],
			"domains" : [ "cppblog" ]
		},
		"locales" : [
			// Select the one you need 
			"en_US.UTF-8",
			// "he_IL.UTF-8",
			// "zh_CN.UTF-8",
		]
	},
	"security" : {
		"csrf" : {
			"enable" : true,
			"exposed": true
		}
	}
}
