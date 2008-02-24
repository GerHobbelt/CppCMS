drop table if exists users;
drop table if exists posts;
drop table if exists comments;
drop table if exists options;

create table users (
	id  integer  primary key autoincrement not null,
	username varchar(32) unique not null,
	password varchar(32) not null
);

create table posts (
	id integer  primary key autoincrement not null,
	author_id integer not null,
	title varchar(256) not null,
	abstact text not null,
	content text,
	publish datetime not null,
	is_open integer not null
);
create index posts_pub on posts (publish);
create index posts_open on posts (is_open);

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

