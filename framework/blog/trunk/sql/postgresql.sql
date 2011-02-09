begin;

drop table if exists pages;
drop table if exists links;
drop table if exists link_cats;
drop table if exists comments;
drop table if exists post2cat;
drop table if exists cats;
drop table if exists posts;
drop table if exists users;
drop table if exists text_options;



create table users (
	id  serial  primary key not null,
	username varchar(32) unique not null,
	password varchar(32) not null
);

create table posts (
	id serial  primary key  not null,
	author_id integer not null references users(id),
	title varchar(256) not null,
	abstract text not null,
	content text not null,
	publish timestamp not null,
	comment_count integer not null default 0,
	is_open integer not null
);
create index posts_pub on posts (is_open,publish);

create table comments (
	id serial primary key not null,
	post_id integer not null references posts(id),
	author varchar(64) not null,
	email  varchar(64) not null,
	url    varchar(128) not null,
	publish_time timestamp not null,
	content text not null
);
create index comments_ord on comments (post_id,publish_time);

create table text_options (
	id varchar(64) primary key not null,
	value text not null
);

create table cats (
	id serial  primary key  not null,
	name varchar(64) not null
);

create table post2cat (
	post_id integer not null references posts(id),
	cat_id integer not null references cats(id),
	publish timestamp not null,
	is_open integer not null,
	unique (post_id,cat_id)
);

create index posts_in_cat on post2cat (is_open,cat_id,publish);

create table link_cats (
	id serial  primary key  not null,
	name varchar(128) not null
);

create table links (
	id serial  primary key  not null,
	cat_id integer not null references link_cats(id),
	title varchar(128) not null,
	url varchar(128) not null,
	description text not null
);

create table pages (
	id serial  primary key  not null,
	author_id integer not null,
	title varchar(256) not null,
	content text not null,
	is_open integer not null
);

insert into text_options values('dbversion','2');

commit;
