begin;

drop table if exists pages;
drop table if exists links;
drop table if exists link_cats;
drop table if exists post2cat;
drop table if exists cats;
drop table if exists text_options;


alter table posts add column comment_count integer not null default 0;

update posts set comment_count=(select count(*) from comments where post_id=posts.id);

create table text_options (
	id varchar(64) primary key not null,
	value text not null
) Engine = InnoDB;

create table cats (
	id integer auto_increment primary key not null,
	name varchar(64) not null
) Engine = InnoDB;

create table post2cat (
	post_id integer not null references posts(id),
	cat_id integer not null references cats(id),
	publish datetime not null,
	is_open integer not null,
	constraint unique (post_id,cat_id),
	FOREIGN KEY (post_id) REFERENCES posts(id),
	FOREIGN KEY (cat_id) REFERENCES cats(id)
) Engine = InnoDB;
create index posts_in_cat on post2cat (is_open,cat_id,publish);


create table link_cats (
	id integer auto_increment primary key not null,
	name varchar(128) not null
) Engine = InnoDB ;

create table links (
	id integer auto_increment primary key not null,
	cat_id integer not null references link_cats(id),
	title varchar(128) not null,
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
	FOREIGN KEY (author_id) REFERENCES users(id)
) Engine = InnoDB;

insert into text_options(id,value) values('dbversion','1');

commit;
