{
	"wikipp" : {
		"script" : "/wikipp",
		"media" :"/templates",
		"syntax_highlighter" : "/templates/sh",
		"disable_registration" : false,
		"languages" : {
			"en" : "en_US" ,
			"he" : "he_IL"
		},

		"sql" : {
			"driver" : "sqlite3", // "mysql", "postgresql"
			"params" : {
				"dbname" : "wikipp.db",
				"sqlite3_dbdir" : "./db/"
			}
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
		"api" : "http",
		"port" : 8080
	},
	"session" : {
		"expire" : "renew",
		"location" : "client",
		"cookies" :  {
			"prefix" : "wikipp"
		},
		"client" : {
			"encryptor" : "aes",
			"key" : "9bc6dbda707cb72ea1205dd5b1c90464"
		}
	},
	"views" : {
		//"default_skin" : "view",
		"paths" : [ "./templates/.libs"] ,
		"skins" : [ "view" ]
	},
	"file_server" : {
		"enable": true,
		"doument_root" : "."
	},
	"localization" : {
		"encoding" : "UTF-8",
		"messages" : { 
			"paths" : [ "./locale"],
			"domains" :  [ "wikipp" ]
		},
		"locales" : [ "he_IL" , "en_US" ]
	},
	"http" : {
		"script_names" : [ "/wikipp" ]
	},
	"logging" : {
		"level" : "debug" 
	}

}



//# Localization
//locale.dir = "./locale"
//locale.dir = "/usr/local/share/locale"
//locale.lang_list = { "he" "en" }
//locale.lang_default = "en"
//locale.domain_list = { "wikipp" }





