create table users (
	id  serial  primary key not null,
	username varchar(32) unique not null,
	password varchar(32) not null
);

create table posts (
	id serial  primary key  not null,
	author_id integer not null,
	title varchar(256) not null,
	abstract text not null,
	content text not null,
	publish timestamp not null,
	is_open integer not null
);
create index posts_pub on posts (is_open,publish);

create table comments (
	id serial primary key not null,
	post_id integer not null,
	author varchar(64) not null,
	email  varchar(64) not null,
	url    varchar(128) not null,
	publish_time timestamp not null,
	content text not null
);
create index comments_ord on comments (post_id,publish_time);

create table options (
	id integer unique primary key not null,
	value varchar(128) not null
);

