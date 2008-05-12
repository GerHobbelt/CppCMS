drop table if exists pages;
drop table if exists links;
drop table if exists link_cats;
drop table if exists options;
drop table if exists comments;
drop table if exists post2cat;
drop table if exists cats;
drop table if exists posts;
drop table if exists users;

create table users (
	id  integer  primary key autoincrement not null,
	username varchar(32) unique not null,
	password varchar(32) not null
);

create table posts (
	id integer  primary key autoincrement not null,
	author_id integer not null,
	title varchar(256) not null,
	abstract text not null,
	content text not null,
	publish datetime not null,
	is_open integer not null
);
create index posts_pub on posts (is_open,publish);

create table comments (
	id integer primary key autoincrement not null,
	post_id integer not null,
	author varchar(64) not null,
	email  varchar(64) not null,
	url    varchar(128) not null,
	publish_time datetime not null,
	content text not null
);
create index comments_ord on comments (post_id,publish_time);

create table options (
	id integer unique primary key not null,
	value varchar(128) not null
);

create table cats (
	id integer auto_increment primary key not null,
	name varchar(64) not null
);

create table post2cat (
	post_id integer not null,
	cat_id integer not null,
	publish datetime not null,
	is_open integer not null
);

create index posts_in_cat on post2cat (is_open,cat_id,publish);


create table link_cats (
	id integer primary key autoincrement not null,
	name varchar(128) not null
);

create table links (
	id integer primary key autoincrement not null,
	cat_id integer not null,
	title varchar(128) unique not null,
	url varchar(128) not null,
	description text not null
);

create table pages (
	id integer primary key autoincrement not null,
	author_id integer not null,
	title varchar(256) not null,
	content text not null,
	is_open integer not null,
	is_rtl integer not null default 0
);
