drop table if exists pages;
drop table if exists links;
drop table if exists link_cats;
drop table if exists options;
drop table if exists comments;
drop table if exists posts;
drop table if exists users;

create table users (
	id  integer auto_increment primary key not null,
	username varchar(32) unique not null,
	password varchar(32) not null
) Engine = InnoDB;

create table posts (
	id integer  auto_increment primary key not null,
	author_id integer not null,
	title varchar(256) not null,
	abstract text not null,
	content text not null,
	publish datetime not null,
	is_open integer not null,
	is_rtl integer not null default 0,
	FOREIGN KEY (author_id) REFERENCES users(id)
) Engine = InnoDB;
create index posts_pub on posts (is_open,publish);

create table comments (
	id integer auto_increment primary key not null,
	post_id integer not null references posts(id),
	author varchar(64) not null,
	email  varchar(64) not null,
	url    varchar(128) not null,
	publish_time datetime not null,
	content text not null,
	FOREIGN KEY (post_id) REFERENCES posts(id)
) Engine = InnoDB;
create index comments_ord on comments (post_id,publish_time);

create table options (
	id integer unique primary key not null,
	value varchar(128) not null
) Engine = InnoDB;

create table link_cats (
	id integer auto_increment primary key not null,
	name varchar(128) not null
) Engine = InnoDB ;

create table links (
	id integer auto_increment primary key not null,
	cat_id integer not null references link_cats(id),
	title varchar(128) unique not null,
	url varchar(128) not null,
	description text not null,
	FOREIGN KEY (cat_id) REFERENCES link_cats(id)
) Engine = InnoDB;

create table pages (
	id integer  auto_increment primary key not null,
	author_id integer not null,
	title varchar(256) not null,
	content text not null,
	is_open integer not null,
	is_rtl integer not null default 0,
	FOREIGN KEY (author_id) REFERENCES users(id)
) Engine = InnoDB;
