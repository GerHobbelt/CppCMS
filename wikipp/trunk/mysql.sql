drop table if exists history;
drop table if exists pages;
drop table if exists connections;
drop table if exists users;

create table users (
	id integer auto_increment primary key not null,
	username varchar(32) unique not null,
	password varchar(32) not null
) Engine = InnoDB;

create table pages (
	id integer auto_increment primary key not null,
	lang varchar(16) not null,
	slug varchar(128) not null,
	title varchar(256) not null,
	content text not null,
	sidebar text not null,
	users_only integer not null,
	constraint unique (lang,slug)
) Engine = InnoDB;

create table connections (
	lang varchar(16) not null,
	slug varchar(128) not null,
	target_id integer not null,
	foreign key(target_id) references pages(id)
) Engine = InnoDB;

create index connections_ind on connections(lang,slug);

create table history (
	id integer not null,
	version integer auto_increment not null,
	created datetime not null,
	title varchar(256) not null,
	content text not null,
	sidebar text not null,
	constraint unique(id,version),
	foreign key(id) references pages(id),
	primary key(id,version)
) Engine = InnoDB;


