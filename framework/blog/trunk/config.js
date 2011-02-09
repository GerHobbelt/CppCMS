{
	
	"blog" : {
		"media" : "/media",
		"root" : "/blog",
		"host" : "localhost:8080",
		"connection_string" : "sqlite3:db=cppcms.db;@pool_size=10"
		//"connection_string" : "mysql:database=cppcms;user=root;password=root;@pool_size=10;@use_prepared=on"
	},
	"service" : {
		"api" : "http",
		"port" : 8080
	},
	"http" : {
		"script" : "/blog" 
	},
	"views" : {
		 "default_skin" : "contendend" ,
		 "paths" : [ "./" ],
		 "skins" : [ "contendend" ],
	},
	"file_server" : {
		"enable" : true,
		"document_root" : "../"
	},
	"session" : {
		"expire" : "renew",
		"timeout" : 604800,
		"location" : "client",
		"client" : {
			"hmac" : "sha1",
			"hmac_key" : "232074faa0fd37de20858bf8cd0a7d10"
		}
	},
	"cache" : {
		"backend" : "thread_shared",
		"limit" : 100,
	},
	"localization" : {
		// "backend" : "std",
		"messages" : {
			"paths" : [ "./locale" ],
			"domains" : [ "blog" ]
		},
		"locales" : [ 
			"en_US.UTF-8",
			"he_IL.UTF-8"
		]
	}
}
