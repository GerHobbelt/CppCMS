{
	"wikipp" : {
		"script" : "/wikipp",
		"media" :"/templates",
		"syntax_highlighter" : "/templates/sh",
		"disable_registration" : false,
		"languages" : {
			"en" : "en_US" ,
			"he" : "he_IL" ,
			"ru" : "ru_RU" 
		},

		"sql" : {
			//"driver" : "sqlite3", // "mysql", "postgresql"
			"driver" : "mysql", // "mysql", "postgresql"
			//"params" : {"dbname" : "wikipp.db","sqlite3_dbdir" : "./db/"}
			"params" : {"dbname" : "wikipp","username" : "root", "password" : "root" } // mysql
		}
		//	dbname="wikipp"
		//	username="root"
		//	password="root"

		//	sqlite3.dbname="wikipp.db"
		//	sqlite3.sqlite3_dbdir="./db/"

		//	postgresql.dbname="cppcms"
		//	postgresql.username="artik"
	},
	"service" : {
		//"worker_processes" : 1,
		"worker_threads" : 5,
		//"api" : "http",
		"api" : "fastcgi",
		//"port" : 8081
		//"socket" : "/tmp/wikipp.sock"
		"socket" : "stdin"
	},
	"session" : {
		"expire" : "renew",
		"location" : "client",
		"timeout" : 100000,
		"cookies" :  {
			"prefix" : "wikipp"
		},
		"server" : {
			"storage" : "files" 
		},
		"client" : {
			"encryptor" : "aes",
			"key" : "9bc6dbda707cb72ea1205dd5b1c90464"
		}
	},
	"views" : {
		//"default_skin" : "view",
		"paths" : [ "/home/artik/Projects/FastBlog/cppblog/wikipp_v100/stage"] ,
		"skins" : [ "view" ] ,
		//"auto_reload" : true
	},
	"file_server" : {
		"enable": true,
		"doument_root" : "."
	},
	"localization" : {
		"encoding" : "UTF-8",
		"messages" : { 
			"paths" : [ "/home/artik/Projects/FastBlog/cppblog/wikipp_v100/stage/locale"],
			"domains" :  [ "wikipp" ]
		},
		"locales" : [ "he_IL" , "en_US" ]
	},
	"http" : {
		//"script_names" : [ "/wikipp" ]
	},
	"logging" : {
		"level" : "info",
		"syslog" : {
			"enable": true,
			"id" : "WikiPP"
		},
		"file" : {
			"name" : "/home/artik/Projects/FastBlog/cppblog/wikipp_v100/wikipp.log",
			"append" : true
		}
	},
	"cache" : {
		"backend" : "thread_shared", 
		"limit" : 100, // items - thread cache
	}

}



//# Localization
//locale.dir = "./locale"
//locale.dir = "/usr/local/share/locale"
//locale.lang_list = { "he" "en" }
//locale.lang_default = "en"
//locale.domain_list = { "wikipp" }





