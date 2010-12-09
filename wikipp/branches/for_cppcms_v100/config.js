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

		"connection_string" : "mysql:database=cppcms;user=root;password=root",

		// "dbixx_conn" : "mysql:dbname=wikipp;username=root;password=root",
		// "dbixx_conn" : "postgresql:dbname=cppcms;username=artik",
		
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
		"level" : "info",
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
		//"backend" : "thread_shared", 
		"limit" : 100, // items - thread cache
	}

}



