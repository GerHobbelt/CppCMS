DROP TABLE IF EXISTS cb_options;

CREATE TABLE cb_options
(
	name	varchar(32) PRIMARY KEY NOT NULL,
	value	varchar(256) NOT NULL
);

DROP TABLE IF EXISTS cb_users;

CREATE TABLE cb_users
(

	id integer PRIMARY KEY NOT NULL AUTO_INCREMENT,
	name varchar(64) unique not null,
	password varchar(16) not null,
	cookie_password varchar(16) not null
);

DROP TABLE IF EXISTS cb_posts;
CREATE TABLE cb_posts
(
	id integer PRIMARY KEY AUTO_INCREMENT NOT NULL,
	author_id integer not NULL,
	state enum('draft','published') not null,
	content_type enum('html','pain') not null,
	direction enum('ltr','rtl') not null,
	title varchar(128) not null,
	abstract text not null,
	content  mediumtext not null,
	published datetime not null,
	index (published),
	index (author_id,id)
);

DROP TABLE IF EXISTS cb_comments;

CREATE TABLE cb_comments
(
	id integer PRIMARY KEY NOT NULL AUTO_INCREMENT,
	post_id integer not null,
	author_id integer not null,
	name varchar(64) not null,
	email varchar(64) not null,
	url varchar(128) not null,
	approved integer not null,
	content text not null,
	index(post_id,id)
);

DROP TABLE IF EXISTS cb_approved_comments;
CREATE TABLE cb_approved_comments
(
	name varchar(64) not null,
	email varchar(64) not null,
	moderator integer not null,
	constraint UNIQUE (name,email,moderator)
);


DROP TABLE IF EXISTS cb_categories;
CREATE TABLE cb_categories
(
	id integer primary key auto_increment not null,
	name varchar(64) not null
);

DROP TABLE IF EXISTS cb_post2cat;
CREATE TABLE cb_post2cat
(
	post_id integer not null,
	cat_id integer not null,
	publish_time datetime not null,
	constraint isset UNIQUE(post_id,cat_id),
	index(cat_id,publish_time)
);

