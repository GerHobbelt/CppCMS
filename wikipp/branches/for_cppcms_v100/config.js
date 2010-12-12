{
	"wikipp" : {
		"script" : "/wikipp",
		"media" :"/templates",
		"syntax_highlighter" : "/templates/sh",
		"disable_registration" : false,

		"languages" : {
			"en" : "en_US.UTF-8" ,
			"he" : "he_IL.UTF-8" ,
			"ru" : "ru_RU.UTF-8" ,
			"pl" : "pl_PL.UTF-8"
		},

		//"connection_string" : "sqlite3:db=./db/wikipp.db",
		"connection_string" : "postgresql:dbname=wikipp",
		//"connection_string" : "mysql:database=wikipp;user=root;password=root",

		//"connection_string" : "sqlite3:db=./db/wikipp.db;@use_prepared=off",
		//"connection_string" : "postgresql:dbname=wikipp;@use_prepared=off",
		//"connection_string" : "mysql:database=wikipp;user=root;password=root;@use_prepared=off",
		
		//"connection_string" : "odbc:@engine=sqlite3;Driver=SQlite3;Database=./db/wikipp.db",
		//"connection_string" : "odbc:@engine=postgresql;Driver=PostgreSQL ANSI;Database=wikipp",
		//"connection_string" : "odbc:@engine=mysql;Driver=MySQL;Database=wikipp;UID=root;PWD=root",
		
	},
	"service" : {
		//"worker_processes" : 1,
		"worker_threads" : 25,
		"api" : "http",
		//"api" : "fastcgi",
		"port" : 8080
		//"socket" : "/tmp/wikipp.sock"
		//"socket" : "stdin"
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
		"paths" : [ "./stage"] ,
		"skins" : [ "view" ] ,
		//"auto_reload" : true
	},
	"file_server" : {
		"enable": true,
		"doument_root" : "."
	},
	"localization" : {
		"backend" : "std" ,
		"messages" : { 
			"paths" : [ "./stage/locale"],
			"domains" :  [ "wikipp" ]
		},
		"locales" : [ "he_IL.UTF-8" , "en_US.UTF-8", "ru_RU.UTF-8", "pl_PL.UTF-8" ]
	},
	"http" : {
		"script_names" : [ "/wikipp" ]
	},
	"logging" : {
		"level" : "error",
		"syslog" : {
			"enable": true,
			"id" : "WikiPP"
		},
		"file" : {
			"name" : "./wikipp.log",
			"append" : true
		}
	},
	"cache" : {
		"backend" : "thread_shared", 
		"limit" : 100, // items - thread cache
	},
	"security" : {
		// "multipart_form_data_limit" : 65536, // KB
		// "content_length_limit" : 1024, // KB
		// "uploads_path" : "" // temporary directory
		"display_error_message" : false
	}

}



